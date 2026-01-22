# EthernetInterface Notes

## Build Flags

- `-DWIFI_AT_ENABLED=1|0` (default `0` on STM32, `1` elsewhere): Enables the ESP-AT WiFi stack.
- `-DSTRINGFORMATTER_FLOATS=1|0` (default `0`): Enables `%f` handling in `StringFormatter`.

## Config Defines

Defined in `src/NetworkConfig.h`:

- `WIFI_AT_ENABLED`: Build-time switch for WiFi AT support.
- `STRINGFORMATTER_FLOATS`: Build-time switch for float printing in `StringFormatter`.
- `DCCEX_ENABLED` (commented by default): Enables CommandStation-EX integration.
- `CLI_ENABLED`: Enables the CLI code path (if present in the build).

## Library Notes

- `lib/WiFiEspAT` is a local copy (patched) to avoid enum name collisions on STM32.
- STM32 builds use `STM32Ethernet` + `STM32duino LwIP`.

