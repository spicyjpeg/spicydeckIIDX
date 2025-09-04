
# IOP firmware

This directory contains the source code for the firmware that runs on the "IOP"
chip. The IOP is a [CH32V003F4P6](https://www.wch-ic.com/products/CH32V003.html)
microcontroller used as an input expander, handling all analog controls
(potentiometers) as well as each deck's buttons and communicating with the ESP32
over I2C. The IOP has I2C address `0x10` and shares its bus with the right
deck's AS5600 encoder.

## GPIO pin assignment

| Pin | Dir | Peripheral | Description               |
| --: | :-- | :--------- | :------------------------ |
| PA1 | I   | ADC1       | Right filter knob         |
| PA2 | I   | ADC0       | Left filter knob          |
| PC0 | IO  |            | Button matrix row 0       |
| PC1 | IO  | I2C        | To right deck's I2C SDA   |
| PC2 | I   | I2C        | To right deck's I2C SCL   |
| PC3 | O   |            | Button matrix row 1       |
| PC4 | I   | ADC2       | Left speed slider         |
| PC5 | O   |            | Button matrix row 2       |
| PC6 | O   |            | Button matrix row 3       |
| PC7 | O   |            | Button matrix row 4       |
| PD0 | I   |            | Button matrix column 0    |
| PD1 |     |            | _Reserved for programmer_ |
| PD2 | I   | ADC3       | Right speed slider        |
| PD3 | I   | ADC4       | Main volume knob          |
| PD4 | I   | ADC7       | Effect depth knob         |
| PD5 | I   | ADC5       | Monitor volume knob       |
| PD6 | I   | ADC6       | Crossfade slider          |
| PD7 | I   |            | Button matrix column 1    |

```
                       ┌────────────┐
   Effect depth  PD4 ▶═╡ ●          ╞═◀ PD3  Main volume
 Monitor volume  PD5 ▶═╡            ╞═◀ PD2  Right speed
      Crossfade  PD6 ▶═╡            ╞═◆ PD1
Matrix column 1  PD7 ▶═╡  CH32V003  ╞═▶ PC7  Matrix row 4
   Right filter  PA1 ▶═╡    F4P6    ╞═▶ PC6  Matrix row 3
    Left filter  PA2 ▶═╡  TSSOP-20  ╞═▶ PC5  Matrix row 2
                 GND ══╡            ╞═◀ PC4  Left speed
Matrix column 0  PD0 ▶═╡            ╞═▶ PC3  Matrix row 1
                3.3V ══╡            ╞═◀ PC2  IOP SCL
   Matrix row 0  PC0 ◆═╡            ╞═◆ PC1  IOP SDA
                       └────────────┘
```

The button matrix consists of the following buttons:

| Row | Column 0           | Column 1            |
| :-- | :----------------- | :------------------ |
|   0 | Left loop in       | Right loop in       |
|   1 | Left loop out      | Right loop out      |
|   2 | Left reloop        | Right reloop        |
|   3 | Left play          | Right play          |
|   4 | Left monitor/shift | Right monitor/shift |

**NOTE**: if `ENABLE_LOGGING=1` is uncommented from the build script, PD5 will
be repurposed as a serial TX pin to output log messages. Logging was used during
development to troubleshoot the I2C state machine; enabling it will slow down
I2C handling to the point of completely breaking it if the bus speed is higher
than a few kilohertz.

## Building the firmware

### Installing dependencies

The following dependencies are required in order to compile the IOP firmware:

- CMake 3.25 or later;
- [Ninja](https://ninja-build.org/);
- a recent GCC toolchain configured for the `riscv-none-elf` target triplet and
  a copy of `libgcc.a` compiled for the `rv32ec` architecture specifically
  (the RISC-V toolchains available in most distros' package managers generally
  ship with a generic `libgcc.a` which is not suitable for the CH32V003).

A suitable toolchain may be spawned by building the latest versions of binutils
and GCC manually from source, passing the following options to their `configure`
scripts respectively:

```bash
# binutils
./configure \
	--target=riscv-none-elf \
	--disable-docs \
	--disable-nls \
	--disable-werror

# GCC
./configure \
	--target=riscv-none-elf \
	--with-arch=rv32ec \
	--with-abi=ilp32e \
	--disable-docs \
	--disable-nls \
	--disable-werror \
	--disable-libada \
	--disable-libssp \
	--disable-libquadmath \
	--disable-threads \
	--disable-libgomp \
	--disable-libstdcxx-pch \
	--disable-hosted-libstdcxx \
	--enable-languages=c,c++ \
	--without-isl \
	--without-headers \
	--with-gnu-as \
	--with-gnu-ld
```

### Compiling and flashing

With CMake and the toolchain installed, building the firmware should be as
simple as running the following commands from the project's `iop` subfolder:

```bash
cmake --preset release
cmake --build build
```

If the toolchain is not listed in your `PATH` environment variable, you will
have to pass the path to its `bin` subdirectory to the first command via the
`-DTOOLCHAIN_PATH` option, like this:

```bash
cmake --preset release -DTOOLCHAIN_PATH=/opt/riscv-none-elf/bin
```

A successful build will produce a `main.bin` file, which you will have to
manually flash to your CH32V003 at address `0x00000000` (or `0x08000000`). This
can be done through any of the programming tools available for the chip, such as
the ones listed below:

- the official WCH-Link dongle, using either WCH's own software or
  [`minichlink`](https://github.com/cnlohr/ch32fun/tree/master/minichlink);
- [`esp32s2-funprog`](https://github.com/cnlohr/esp32s2-cookbook/tree/master/ch32v003programmer);
- [`picorvd`](https://github.com/aappleby/picorvd) (RP2040 based);
- [Ardulink](https://gitlab.com/BlueSyncLine/arduino-ch32v003-swio) (AVR based,
  not recommended as most AVR boards are 5V only);
- [the `rv003usb` programmer](https://github.com/cnlohr/rv003usb/tree/master/rvswdio_programmer)
  (based on *another* CH32V003).

## See also

Some of the code in this folder, particularly the build scripts and C standard
library, has been reused and adapted from
[ps1-bare-metal](https://github.com/spicyjpeg/ps1-bare-metal).
