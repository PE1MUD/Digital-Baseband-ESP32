# Digital-Baseband-ESP32
The Digital Basebands ESP32 source files, version 1.0.

---------------------------------------------------------------------------------------
BB-Controller
(c) PE1MUD, PE1OBW 2024
---------------------------------------------------------------------------------------
For all graphics functions the following library needs to be installed:

https://github.com/Bodmer/TFT_eSPI

The version this was built and tested with is 2.5.43
No other libraries that aren't part of the Arduino dev environment have been used.
Note: some settings need to be changed that are part of the TFT_eSPI library!
Edit User_Setup.h in user\Documents\Arduino\libraries\TFT_eSPI to use the following pins for the display:

#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5  // Chip select control pin
#define TFT_DC   17  // Data Command control pin
#define TFT_RST  16  // Reset pin (could connect to RST pin)

The speed needs a change too, so you will have silky smooth PPM Meters.
For this, select ONLY this line:

#define SPI_FREQUENCY  55000000 // STM32 SPI1 only (SPI2 maximum is 27MHz)

Finally, to stop all the whining about not having a touch screen, uncomment:

#define DISABLE_ALL_LIBRARY_WARNINGS
---------------------------------------------------------------------------------------
Configuring Arduino for the ESP32 that is in use:

Tools->Boards-> (search for) esp32 -> "ESP32 Dev Module" and select this.

The settings for this board:

Frequency: "240MHz"
Core Debug Level: "None"
Erase All Flash....: Disabled
Events Run On: "Core1"
Flash Frequency: "80MHz"
Flash Mode: "QIO"
Flash Size: "4MB (32Mb)"
JTAG Adapter: "Disabled"
Arduino Runs On: "Core1"
Partition Scheme: "Huge APP (3MB no OTA/1MB SPIFFS)"
PSRAM: "Disabled"
Upload Speed: "921600"

This disables the option of OTA updates, but provides 3MB of flash!
Indeed, much memory is needed!
---------------------------------------------------------------------------------------
IMPORTANT NOTICE

You're allowed to use all associated sources, but if and when you
modify anything, you are required to disclose your modifications
in a project (branch). This project should be complete in a way 
that it can be compiled. Should you change anything major 
hardware-wise you must provide adequate details on your 
modifications in a read.me or by any other appropriate method.

If you find bugs, errors, inconsistencies, please provide feedback
to David PE1MUD, or Werner PE1OBW.

Do not remove headers or copyright notices.
---------------------------------------------------------------------------------------
