/**
 * @file  main.c
 * @brief ESP32 – SPI ADC Master (ESP-IDF)
 *
 * Waits for the Arduino Nano slave to assert DATA_READY (D4, active-low), then
 * reads a 10-byte SPI packet and decodes four 13-bit oversampled ADC channels.
 *
 * ─── Packet layout (10 bytes, little-endian) ─────────────────────────────────
 *  [0]     0xAA             Sync / header byte
 *  [1:2]   CH0  uint16_t   A0 oversampled (13-bit, 0–8191)
 *  [3:4]   CH1  uint16_t   A1 oversampled
 *  [5:6]   CH2  uint16_t   A2 oversampled
 *  [7:8]   CH3  uint16_t   A3 oversampled
 *  [9]     uint8_t         XOR checksum of bytes [0..8]
 *
 * ─── Pinout ──────────────────────────────────────────────────────────────────
 *  ESP32 GPIO4   ← Nano D4   DATA_READY  (active-low input, interrupt)
 *  ESP32 GPIO5   → Nano D10  SPI CS      (active-low, managed by IDF driver)
 *  ESP32 GPIO18  → Nano D13  SPI SCK
 *  ESP32 GPIO19  ← Nano D12  SPI MISO
 *  ESP32 GPIO23  → Nano D11  SPI MOSI    (Nano ignores MOSI bytes)
 *  GND           ↔ Nano GND              (common ground – required)
 *
 * All lines pass through a level shifter (3.3 V ↔ 5 V).
 *
 * ─── SPI settings ────────────────────────────────────────────────────────────
 *  Mode 3 (CPOL=1, CPHA=1), MSB first, 4 MHz
 *  SCK idles HIGH – required by the level shifter in use.
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "adc_master";

// ── Pin definitions ───────────────────────────────────────────────────────────

#define DATA_READY_PIN  GPIO_NUM_4
#define SPI_CS_PIN      GPIO_NUM_5
#define SPI_SCK_PIN     GPIO_NUM_18
#define SPI_MISO_PIN    GPIO_NUM_19
#define SPI_MOSI_PIN    GPIO_NUM_23

// ── SPI / packet constants ────────────────────────────────────────────────────

#define SPI_HOST_ID     SPI2_HOST
#define SPI_CLOCK_HZ    (4 * 1000 * 1000)   // 4 MHz; max safe on breadboard

#define PACKET_SIZE     10
#define PACKET_HEADER   0xAAu

// ── Globals ───────────────────────────────────────────────────────────────────

static SemaphoreHandle_t  s_data_ready_sem;
static spi_device_handle_t s_spi;

// ── DATA_READY ISR ────────────────────────────────────────────────────────────

static void IRAM_ATTR data_ready_isr(void *arg)
{
    BaseType_t higher_woken = pdFALSE;
    xSemaphoreGiveFromISR(s_data_ready_sem, &higher_woken);
    if (higher_woken) portYIELD_FROM_ISR();
}

// ── Packet helpers ────────────────────────────────────────────────────────────

static uint8_t xor_checksum(const uint8_t *buf, int len)
{
    uint8_t c = 0;
    for (int i = 0; i < len; i++) c ^= buf[i];
    return c;
}

/**
 * Perform one SPI transaction and decode the result into @p ch[4].
 * Returns true on success, false on header or checksum mismatch.
 */
static bool read_packet(uint16_t ch[4])
{
    uint8_t rx[PACKET_SIZE];
    uint8_t tx[PACKET_SIZE];
    memset(tx, 0x00, sizeof(tx));   // dummy bytes; Nano ignores MOSI

    spi_transaction_t t = {
        .length    = PACKET_SIZE * 8,   // total bits to clock
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    esp_err_t err = spi_device_polling_transmit(s_spi, &t);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI error: %s", esp_err_to_name(err));
        return false;
    }

    if (rx[0] != PACKET_HEADER) {
        ESP_LOGE(TAG, "Bad header: 0x%02X (expected 0x%02X)", rx[0], PACKET_HEADER);
        return false;
    }

    uint8_t expected_chk = xor_checksum(rx, 9);
    if (rx[9] != expected_chk) {
        ESP_LOGE(TAG, "Checksum mismatch: got 0x%02X, expected 0x%02X",
                 rx[9], expected_chk);
        return false;
    }

    // Decode little-endian uint16_t channel values
    ch[0] = (uint16_t)rx[1] | ((uint16_t)rx[2] << 8);
    ch[1] = (uint16_t)rx[3] | ((uint16_t)rx[4] << 8);
    ch[2] = (uint16_t)rx[5] | ((uint16_t)rx[6] << 8);
    ch[3] = (uint16_t)rx[7] | ((uint16_t)rx[8] << 8);

    return true;
}

// ── app_main ──────────────────────────────────────────────────────────────────

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 SPI ADC Master starting");

    // ── Semaphore: ISR → main task signalling ──────────────────────────────────
    s_data_ready_sem = xSemaphoreCreateBinary();
    configASSERT(s_data_ready_sem);

    // ── SPI bus ────────────────────────────────────────────────────────────────
    spi_bus_config_t bus_cfg = {
        .mosi_io_num   = SPI_MOSI_PIN,
        .miso_io_num   = SPI_MISO_PIN,
        .sclk_io_num   = SPI_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST_ID, &bus_cfg, SPI_DMA_CH_AUTO));

    // ── SPI device (Nano slave) ────────────────────────────────────────────────
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = SPI_CLOCK_HZ,
        .mode           = 3,            // CPOL=1, CPHA=1 – matches Nano firmware
        .spics_io_num   = SPI_CS_PIN,   // IDF asserts/deasserts CS automatically
        .queue_size     = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_ID, &dev_cfg, &s_spi));

    // ── DATA_READY GPIO interrupt (falling edge) ───────────────────────────────
    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << DATA_READY_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,     // redundant if level shifter has
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // its own pull-up, but safe
        .intr_type    = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_cfg));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(DATA_READY_PIN, data_ready_isr, NULL));

    ESP_LOGI(TAG, "Ready – waiting for Nano DATA_READY");

    // ── Main acquisition loop ──────────────────────────────────────────────────
    while (true) {
        // Block until Nano asserts DATA_READY (ISR gives the semaphore)
        xSemaphoreTake(s_data_ready_sem, portMAX_DELAY);

        uint16_t ch[4];
        if (read_packet(ch)) {
            ESP_LOGI(TAG, "A0=%5u  A1=%5u  A2=%5u  A3=%5u",
                     ch[0], ch[1], ch[2], ch[3]);

            // TODO: add your data handling here (MQTT publish, local storage, etc.)
        }
    }
}
