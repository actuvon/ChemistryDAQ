# ESP32 ADC Master – PlatformIO Commands

## Compile
```
pio run
```

## Upload
```
pio run --target upload
```
Set `upload_port = COMx` in `platformio.ini` if auto-detection fails.

## Serial monitor (ESP-IDF log output)
```
pio device monitor
```
115200 baud. Output tagged `[adc_master]`, e.g.:
```
I (312) adc_master: A0=  4096  A1=  2048  A2=     0  A3=  8191
```

## Clean
```
pio run --target clean
```

## First build note
The first build downloads the ESP-IDF toolchain and generates `sdkconfig` —
it will take several minutes. Subsequent builds are fast.
