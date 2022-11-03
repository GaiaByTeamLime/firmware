# Firmware

This is currently very basic firmware written in C++ together with all kinds of Arduino stuff. 

## How to install

0. Clone this repo
1. Install Arduino IDE, and open firmware.ino
2. Go to Library Manager (the books on the left) and install the following libraries: Adafruit_Unified_Sensor v1.1.6, BH1750 v1.3.0, DHT_sensor_library v1.4.4
3. Add board support for the ESP32 to the arduino IDE: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-arduino-ide
4. Install drivers for the ESP32:

    a. CP2104: [Windows, Mac, Linux](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
    b. CH9102F: [Windows](http://www.wch.cn/downloads/CH34xSerCfg_ZIP.html) [Mac](http://www.wch.cn/downloads/CH34XSER_MAC_ZIP.html) [Linux](http://www.wch.cn/download/CH341SER_LINUX_ZIP.html)

4. Connect your sensor, select the COM port/usb dev thing (you can find that in the CH9102F gui thingy) and select "ESP32 Dev Kit" as the board
5. Click compile+program!