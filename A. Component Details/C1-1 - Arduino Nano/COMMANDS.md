# Nano ADC Slave – PlatformIO Commands

## Compile
```
pio run -e nanoatmega328        # most clones (old STK500v1 bootloader)
pio run -e nanoatmega328new     # genuine post-2018 Nanos (Optiboot)
pio run -e nanoatmega328_debug  # adds Serial.print trace output
```

## Upload
```
pio run -e nanoatmega328 --target upload
```
If upload fails, try `nanoatmega328new` instead.
Upload port is pinned to **COM5** in `platformio.ini` — change `upload_port` there if it moves.

## Serial monitor (debug build only)
```
pio device monitor
```
115200 baud. Shows sampled values and XOR checksum each cycle.

## Clean build artifacts
```
pio run --target clean
```
