/**
 * @file  main.cpp
 * @brief Arduino Nano – SPI ADC Slave (ATmega328P)
 *
 * Continuously oversamples A0–A3, then asserts DATA_READY (D4, active-low) to
 * notify the ESP32 master.  When the ESP32 asserts SS (D10) and clocks the SPI
 * bus, the Nano shifts out a 10-byte packet containing all four channel results.
 *
 * ─── Oversampling ────────────────────────────────────────────────────────────
 *  Standard ATmega oversampling technique:
 *    • Take 4^N samples, accumulate in a 32-bit sum
 *    • Right-shift the sum by N to yield a (10 + N)-bit result
 *  Default: N = 3  →  64 samples per channel  →  13-bit result (0 – 8191)
 *  Approx. update rate at default ADC prescaler (128): ~37 Hz
 *
 * ─── Packet layout (10 bytes, little-endian) ─────────────────────────────────
 *  [0]     0xAA             Sync / header byte
 *  [1:2]   CH0  uint16_t   A0 oversampled value
 *  [3:4]   CH1  uint16_t   A1 oversampled value
 *  [5:6]   CH2  uint16_t   A2 oversampled value
 *  [7:8]   CH3  uint16_t   A3 oversampled value
 *  [9]     uint8_t         XOR checksum of bytes [0..8]
 *
 * ─── Pinout ──────────────────────────────────────────────────────────────────
 *  Nano D4   → ESP32 GPIO    DATA_READY  (active-low output; tie to ESP32 INT)
 *  Nano D10  ← ESP32 SPI CS  SS          (active-low input,  hardware pin)
 *  Nano D11  ← ESP32 MOSI               (input; this firmware ignores MOSI)
 *  Nano D12  → ESP32 MISO               (data output)
 *  Nano D13  ← ESP32 SCK                (clock input)
 *  GND       ↔ ESP32 GND               (common ground – required)
 *
 * ─── SPI settings ────────────────────────────────────────────────────────────
 *  Mode 3 (CPOL = 1, CPHA = 1), MSB first
 *  SCK idles HIGH — required for level shifters that need normally-high comm lines
 *  Recommended master clock: ≤ 4 MHz on breadboard; max is F_CPU/2 = 8 MHz
 *
 * ─── Handshake flow ──────────────────────────────────────────────────────────
 *  Nano                            ESP32
 *  ────                            ─────
 *  samples A0–A3
 *  builds packet
 *  pre-loads SPDR with byte[0]
 *  DATA_READY ──────LOW──────────► interrupt / GPIO detect
 *                                  assert SS (CS) LOW
 *  SPI_STC ISR clocks bytes 1–9 ◄─ clock 10 bytes
 *                                  deassert SS HIGH
 *  PCINT: SS HIGH detected
 *  DATA_READY ──────HIGH─────────► (optional: deassert INT)
 *  (repeat)
 */

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// ── Configuration ─────────────────────────────────────────────────────────────

/** GPIO pin used to signal the ESP32 that a packet is ready (active-low). */
static constexpr uint8_t DATA_READY_PIN = 4;

/**
 * Oversampling depth N.
 *   N = 2 → 16 samples → 12-bit result (0 – 4 095)
 *   N = 3 → 64 samples → 13-bit result (0 – 8 191)  ← default
 *   N = 4 → 256 samples → 14-bit result (0 – 16 383)
 * Increasing N improves resolution but reduces throughput proportionally.
 */
static constexpr uint8_t OVERSAMPLE_BITS = 3;
static constexpr uint8_t NUM_SAMPLES     = 1u << (2u * OVERSAMPLE_BITS);  // 4^N

/** Packet structure constants. */
static constexpr uint8_t PACKET_SIZE   = 10;
static constexpr uint8_t PACKET_HEADER = 0xAAu;

// ── SPI transmit state (shared between loop() and ISRs) ───────────────────────

/** Outgoing packet buffer.  Written by loop(), read by SPI_STC ISR. */
static volatile uint8_t tx_buf[PACKET_SIZE];

/**
 * Index of the NEXT byte to load into SPDR inside SPI_STC_vect.
 * Initialised to 1 because byte [0] is pre-loaded before the transfer starts.
 */
static volatile uint8_t tx_idx = 1;

/**
 * Set to true by PCINT0_vect when SS rises (master deasserts chip-select).
 * Cleared by loop() before each transfer.
 */
static volatile bool xfer_done = false;

// ── ADC oversampling ──────────────────────────────────────────────────────────

/**
 * Collect NUM_SAMPLES readings on @p pin and return the oversampled result.
 * Result is (10 + OVERSAMPLE_BITS) bits wide, stored in a uint16_t.
 */
static uint16_t oversample(uint8_t pin)
{
    uint32_t acc = 0;
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        acc += static_cast<uint32_t>(analogRead(pin));
    }
    // Shift right by N to decimate noise and recover the higher-resolution value.
    return static_cast<uint16_t>(acc >> OVERSAMPLE_BITS);
}

// ── Packet helpers ────────────────────────────────────────────────────────────

static uint8_t xor_checksum(volatile const uint8_t *buf, uint8_t len)
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < len; i++) c ^= buf[i];
    return c;
}

static void build_packet(uint16_t ch0, uint16_t ch1, uint16_t ch2, uint16_t ch3)
{
    tx_buf[0] = PACKET_HEADER;
    tx_buf[1] = static_cast<uint8_t>(ch0);
    tx_buf[2] = static_cast<uint8_t>(ch0 >> 8u);
    tx_buf[3] = static_cast<uint8_t>(ch1);
    tx_buf[4] = static_cast<uint8_t>(ch1 >> 8u);
    tx_buf[5] = static_cast<uint8_t>(ch2);
    tx_buf[6] = static_cast<uint8_t>(ch2 >> 8u);
    tx_buf[7] = static_cast<uint8_t>(ch3);
    tx_buf[8] = static_cast<uint8_t>(ch3 >> 8u);
    tx_buf[9] = xor_checksum(tx_buf, 9);
}

// ── SPI slave init ────────────────────────────────────────────────────────────

static void spi_slave_init()
{
    // Direction: MISO (PB4 = D12) is the only output.
    DDRB |=  (1u << PB4);                               // MISO → output
    DDRB &= ~((1u << PB5) | (1u << PB3) | (1u << PB2)); // SCK, MOSI, SS → inputs

    // Internal pull-up on SS keeps line HIGH while the master is idle, preventing
    // spurious SPI activity if the bus is undriven.
    PORTB |= (1u << PB2);

    // SPCR configuration:
    //   SPIE = 1  — enable SPI transfer-complete interrupt
    //   SPE  = 1  — enable SPI hardware
    //   DORD = 0  — MSB transmitted first
    //   MSTR = 0  — slave mode (bit is already 0 after reset)
    //   CPOL = 1  — clock idle HIGH  (SPI Mode 3)
    //   CPHA = 1  — sample on falling edge
    //   SPR1:0 ignored in slave mode (prescaler applies only to master)
    SPCR = (1u << SPIE) | (1u << SPE) | (1u << CPOL) | (1u << CPHA);
}

// ── ISR: SPI byte-transfer complete ──────────────────────────────────────────
/**
 * Fires after each 8-bit SPI transfer completes.  Load the next byte into SPDR
 * so it is ready when the master begins the next byte.  Pad with 0x00 if the
 * master reads beyond the packet boundary (defensive).
 */
ISR(SPI_STC_vect)
{
    SPDR = (tx_idx < PACKET_SIZE) ? tx_buf[tx_idx++] : 0x00u;
}

// ── ISR: PCINT0 – SS pin edge (PB2 = PCINT2) ─────────────────────────────────
/**
 * Fires on any logic change on the PCIE0 group.  Only PB2 is unmasked, so only
 * SS edges reach here.  A HIGH level means SS was deasserted → transfer is over.
 */
ISR(PCINT0_vect)
{
    if (PINB & (1u << PB2)) {   // SS is now HIGH → rising edge
        xfer_done = true;
    }
}

// ── setup() ───────────────────────────────────────────────────────────────────
void setup()
{
#ifdef DEBUG_SERIAL
    Serial.begin(115200);
    Serial.println(F("=== Nano SPI ADC Slave ==="));
    Serial.print(F("Oversampling: N="));
    Serial.print(OVERSAMPLE_BITS);
    Serial.print(F(", samples="));
    Serial.print(NUM_SAMPLES);
    Serial.print(F(", result bits="));
    Serial.println(10 + OVERSAMPLE_BITS);
#endif

    // DATA_READY: output, idle HIGH (not ready)
    pinMode(DATA_READY_PIN, OUTPUT);
    digitalWrite(DATA_READY_PIN, HIGH);

    // ADC: AVcc (~5 V) reference, default prescaler 128 → 125 kHz ADC clock
    analogReference(DEFAULT);

    // Hardware SPI in slave mode
    spi_slave_init();

    // Pin-change interrupt on SS (PB2) to detect rising edge (transfer done)
    PCICR  |= (1u << PCIE0);    // Enable PCINT for Port B
    PCMSK0 |= (1u << PCINT2);   // Unmask only PCINT2 (PB2 = D10 = SS)

    sei();
}

// ── loop() ────────────────────────────────────────────────────────────────────
void loop()
{
    // ── 1. Oversample all four channels ───────────────────────────────────────
    uint16_t ch[4];
    ch[0] = oversample(A0);
    ch[1] = oversample(A1);
    ch[2] = oversample(A2);
    ch[3] = oversample(A3);

    // ── 2. Pack the results ───────────────────────────────────────────────────
    build_packet(ch[0], ch[1], ch[2], ch[3]);

    // ── 3. Arm SPI: pre-load byte[0] and reset the byte index ────────────────
    //
    //  This block is atomic (cli/sei) to guard against a hypothetical SS glitch
    //  asserting the SPI hardware before we finish setup.  In normal operation
    //  DATA_READY is still HIGH at this point so the master will not assert SS.
    cli();
    tx_idx    = 1;              // SPI_STC ISR will send bytes [1 .. PACKET_SIZE-1]
    SPDR      = tx_buf[0];     // byte[0] pre-loaded; clocked on master's first 8 clocks
    xfer_done = false;
    sei();

    // ── 4. Assert DATA_READY – tell the ESP32 a fresh packet is waiting ───────
    digitalWrite(DATA_READY_PIN, LOW);

#ifdef DEBUG_SERIAL
    Serial.print(F("PKT  A0="));  Serial.print(ch[0]);
    Serial.print(F("  A1="));     Serial.print(ch[1]);
    Serial.print(F("  A2="));     Serial.print(ch[2]);
    Serial.print(F("  A3="));     Serial.print(ch[3]);
    Serial.print(F("  chk=0x")); Serial.println(tx_buf[9], HEX);
#endif

    // ── 5. Wait for the ESP32 to read the packet (SS rising edge via PCINT) ───
    //
    //  The SPI_STC ISR autonomously shifts out bytes; this loop just waits for
    //  the transfer to be declared complete.
    //
    //  NOTE: If the ESP32 never responds, this spins indefinitely.  For a
    //  production build, add a watchdog or software timeout here.
    while (!xfer_done) { /* spin */ }

    // ── 6. Deassert DATA_READY, then immediately start the next acquisition ───
    digitalWrite(DATA_READY_PIN, HIGH);
}
