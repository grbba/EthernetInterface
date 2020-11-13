# How to flash your ESP for the right AT commandset

Download the AT commandset from here [Expressif](https://www.espressif.com/en/support/download/at) download `ESP8266 NonOS AT Bin V1.7.4` you can use also the 2.1 version for your board. The github page from the WiFiEspAT library has also usefull information. Use the following command to flash the ESP. We assume that you know how to put your ESP into flash mode though. If not helpfull information may be found here: **to be done**

```bash

esptool.py write_flash --flash_size 2MB-c1 0x0 boot_v1.7.bin 0x01000 at/1024+1024/user1.2048.new.5.bin 0x1fb000 blank.bin 0x1fc000 esp_init_data_default_v08.bin 0xfe000 blank.bin 0x1fe000 blank.bin

```

Here we use the esptool.py program for flashing but it should work with the flash tool of your choice if you respect the flash map. More information on esptool.py can be found [here](https://github.com/espressif/esptool).