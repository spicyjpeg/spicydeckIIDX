
# spicydeckIIDX

spicydeckIIDX is a minimal, self-contained, fully standalone DJ mixing setup
with dual jog wheels that simulate vinyl turntables using motors with closed
loop feedback.

The hardware consists of:

- an ESP32 as the main microcontroller, alongside a CH32V003F4P6 acting as an
  I/O processor ("IOP");
- two brushed DC motors driving the decks, both in turn driven by a single
  DRV8833 H-bridge;
- two AS5600 magnetic encoders for sensing each deck's speed;
- a 160x128 TFT display (ST7735 controller) for track selection;
- an SD card slot wired up in 1-bit SDIO mode;
- two I2S DACs for the main and monitor (headphone) outputs respectively.

**NOTE**: this project is currently work-in-progress and lacks proper
documentation and a reference design for the hardware. This preliminary version
of the code has been published for reference purposes only.

## GPIO pin assignment

| Pin  | Dir | Peripheral | Description                               |
| ---: | :-- | :--------- | :---------------------------------------- |
|  IO0 | IO  | LEDC       | Display backlight control, boot override  |
|  IO1 | O   | UART0      | _Reserved for debug UART `TX`_            |
|  IO2 | IO  | SDMMC\*    | SD card `DAT`/`D0`                        |
|  IO3 | I   | UART0      | _Reserved for debug UART `RX`_            |
|  IO4 | O   | I2S1       | Audio DAC `SDOUT` (monitor)               |
|  IO5 | O   | SPI3\*     | Display (ST7735) `/CS`                    |
|  IO6 |     | SPI0\*     | _Connected internally to SPI flash `SCK`_ |
|  IO7 |     | SPI0\*     | _Connected internally to SPI flash `D0`_  |
|  IO8 |     | SPI0\*     | _Connected internally to SPI flash `D1`_  |
|  IO9 |     | SPI0\*     | _Connected internally to SPI flash `D2`_  |
| IO10 |     | SPI0\*     | _Connected internally to SPI flash `D3`_  |
| IO11 |     | SPI0\*     | _Connected internally to SPI flash `CMD`_ |
| IO12 | O   | I2S0       | Audio DAC `BCLK` (main and monitor)       |
| IO13 | O   | I2S0       | Audio DAC `LRCK` (main and monitor)       |
| IO14 | O   | SDMMC\*    | SD card `CLK`                             |
| IO15 | O   | SDMMC\*    | SD card `CMD`                             |
| IO16 | IO  | I2C0       | IOP and left deck encoder (AS5600) `SDA`  |
| IO17 | IO  | I2C0       | IOP and left deck encoder (AS5600) `SCL`  |
| IO18 | O   | SPI3\*     | Display (ST7735) `SCL`                    |
| IO19 | O   |            | Display (ST7735) `D/C`                    |
| IO21 | IO  | I2C1       | Right deck encoder (AS5600) `SDA`         |
| IO22 | IO  | I2C1       | Right deck encoder (AS5600) `SCL`         |
| IO23 | O   | SPI3\*     | Display (ST7735) `SDA`/`D0`               |
| IO25 | O   | MCPWM0     | Left deck motor PWM A                     |
| IO26 | O   | MCPWM0     | Left deck motor PWM B                     |
| IO27 | O   | I2S0       | Audio DAC `SDOUT` (main)                  |
| IO32 | O   | MCPWM0     | Right deck motor PWM A                    |
| IO33 | O   | MCPWM0     | Right deck motor PWM B                    |
|  I34 | I   | PCNT0      | Selector encoder A                        |
|  I35 | I   | PCNT0      | Selector encoder B                        |
|  I36 | I   |            | Selector encoder button                   |
|  I39 | I   |            | _Unused_                                  |

Notes:

- **\*** denotes peripherals whose pins cannot be freely moved as they are
  routed through the ESP32's `IO_MUX` rather than the GPIO matrix. See
  [table 4-3](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#table.4.3)
  of the reference manual for details.
- IO0 and IO2 are used as
  [bootstrapping pins](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html).
  The display backlight and SD card will interfere with the way `esptool`
  triggers a reboot into the bootloader, making it more or less impossible to
  reflash the ESP32 without temporarily disconnecting it from everything else.
- Similarly, IO12 should be pulled down on startup in order not to accidentally
  enable 1.8V SPI flash mode.

```
                           ┏━━━━━━━━━━━━━┓
                           ┃             ┃
                       ┏━┯━╉─────────────╊━┯━┓
                 3.3V  ┃○│ ┃             ┃ │○┃  GND
                   EN  ┃○│ ┃    ESP32    ┃ │○┃▶ 23   Display SDA/D0
 Selector button   36 ▶┃○│ ┃             ┃ │○┃◆ 22   Right deck SCL
 Battery voltage   39 ▶┃○│ ┃             ┃ │○┃▶ 1    (Debug TX)
      Selector A   34 ▶┃○│ ┃             ┃ │○┃◀ 3    (Debug RX)
      Selector B   35 ▶┃○│ ┃             ┃ │○┃◆ 21   Right deck SDA
Right deck PWM A   32 ◀┃○│ ┃ .           ┃ │○┃  GND
Right deck PWM B   33 ◀┃○│ ┡━━━━━━━━━━━━━┩ │○┃▶ 19   Display D/C
 Left deck PWM A   25 ◀┃○│ ┆             ┆ │○┃◆ 18   Display SCL
 Left deck PWM B   26 ◀┃○│ └┄┄┄┄┄┄┄┄┄┄┄┄┄┘ │○┃▶ 5    Display /CS
DAC SDOUT (main)   27 ◀┃○│       ┌─┐       │○┃◆ 17   Left deck + IOP SCL
          SD CLK   14 ◀┃○│       │ ▐▋      │○┃◆ 16   Left deck + IOP SDA
        DAC BCLK   12 ◀┃○│       └─┘       │○┃▶ 4    DAC SDOUT (monitor)
                  GND  ┃○│      ┌───┐      │○┃▶ 0    Display backlight
        DAC LRCK   13 ◀┃○│      │   │      │○┃◆ 2    SD DAT/D0
      (Flash D2)    9 ◆┃○│  EN  └───┘   0  │○┃▶ 15   SD CMD
      (Flash D3)   10 ◆┃○│ ┌─┐         ┌─┐ │○┃◆ 8    (Flash D1)
     (Flash CMD)   11 ◆┃○│ │◉│ ├─────┤ │◉│ │○┃◆ 7    (Flash D0)
                   5V  ┃○│ └─┘ │.   .│ └─┘ │○┃◀ 6    (Flash SCK)
                       ┗━┷━━━━━┷━━━━━┷━━━━━┷━┛
```

## Building the firmware

The main firmware can be compiled using either ESP-IDF or the Arduino IDE. Using
ESP-IDF is recommended as Arduino does not allow for fine-grained control of
various SDK features through IDF's `menuconfig` system.

In either case, the IOP's firmware must be built and flashed separately. See the
[IOP README](iop/README.md) for more information.

### Building using ESP-IDF

Follow
[Espressif's own guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation)
on setting up ESP-IDF. Once set up, the firmware can be compiled and flashed by
cloning this repository and executing the build task provided by the ESP-IDF VS
Code extension, or running the following commands from an IDF terminal:

```bash
idf.py build
idf.py -p <port> flash
```

### Building using the Arduino IDE

A recent version of the Arduino IDE with support for the "sketch-in-subfolder"
project layout is required. An up-to-date version of the ESP32 Arduino core must
additionally be installed prior to continuing; refer to the
[installation guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
for details. No external libraries or other dependencies are required.

The contents of this repository shall be cloned into a folder named
`spicydeckIIDX`, which can then be opened directly in the Arduino IDE. The
following settings under the "Tools" menu should be adjusted:

- CPU frequency: **240 MHz**
- Events run on: **Core 0**
- Arduino runs on: **Core 0**
- Partition scheme: **Minimal**
- PSRAM: **Disabled**

The firmware should then build without issues.

## Encoding audio tracks

spicydeckIIDX currently only supports playback of audio files encoded in its own
`.sst` format. This is a custom codec *loosely* inspired by the Sony BRR/CD-XA
ADPCM format, designed to have minimal decoding overhead while providing
features such as the ability to store multiple interleaved "variants" (e.g.
pitch-shifted copies) of the same track and switch between them on the fly.

A simple command line `.sst` encoder is provided under the `tools` directory.
The encoder requires Python 3.10 or later and makes use of a custom extension
module, which in turn has the following dependencies:

- a C++ host compiler toolchain;
- [libKeyFinder](https://mixxxdj.github.io/libkeyfinder);
- [Rubber Band](https://breakfastquay.com/rubberband).

Once said dependencies are satisfied, the module can be built in a Python
virtual environment by running the following commands:

```bash
# Windows (using PowerShell)
py -m venv env
env\Scripts\Activate.ps1
cd tools
py -m pip install -r requirements.txt
py -m Cython.Build.Cythonize -b native.pyx

# Windows (using Cygwin/MSys2), Linux or macOS
python3 -m venv env
source env/bin/activate
cd tools
pip3 install -r requirements.txt
cythonize -b native.pyx
```

Note that even after the virtual environment has been set up, it's **always**
**necessary** to invoke `env\Scripts\Activate.ps1` or `source env/bin/activate`
in any new terminal window prior to running the encoder.
