# Modular Motor Control Firmware

High-performance motor-control firmware organized in layers for portability
across MCUs and inverter boards.

## Directory Structure
```text
motor_firmware/
├── boards/nucleo_f446re/    # NUCLEO-F446RE linker script
├── drivers/
│   ├── drv8312/             # TI DRV8312 bridge driver
│   ├── encoder/             # Encoder driver
│   └── servo/               # SG90 servo driver
├── hal/
│   ├── include/             # MCU-independent HAL interfaces
│   ├── nucleo_f446re/       # NUCLEO-F446RE PWM implementation
│   └── stm32f4/             # STM32F439 HAL implementation and config
├── include/                 # Shared board configuration
├── src/                     # Application entry points
└── third_party/stm32cube/   # Bundled STM32F4 HAL and CMSIS dependencies
```

## Build

Install CMake, GNU Make, and the `arm-none-eabi-gcc` toolchain. The STM32F4 HAL
and CMSIS files needed by this project are bundled in `third_party/stm32cube`;
a separate STM32CubeF4 installation is not required.

Configure and build from the repository root:

```sh
cmake -S . -B build -DCMAKE_C_COMPILER=arm-none-eabi-gcc
cmake --build build --target sg90_nucleo_f446re.elf
```

The NUCLEO-F446RE target produces `build/sg90_nucleo_f446re.elf`, `.hex`, and
`.bin` firmware images. The other NUCLEO-F446RE test targets are
`d9_led_test_nucleo_f446re.elf` and `stepper_28byj48_nucleo_f446re.elf`;
build any target with `cmake --build build --target TARGET_NAME`. Build the
original STM32F439 motor-control archive with
`cmake --build build --target motor_firmware`.

### SG90 wiring and demo

Connect the SG90 signal wire to Arduino **D9 (PC7 / TIM3_CH2)**. Power the
servo from a regulated external 5 V supply rated for at least 1 A; connect the
supply ground to a NUCLEO-F446RE GND pin. Do not power the servo from the
board's 3.3 V output. The servo's red wire is 5 V, brown/black is ground, and
orange/yellow is signal; check the wire colors for your particular servo.

The demo sweeps the servo between 10° and 170° in 1° steps, at 20 ms per step.
The driver generates a 50 Hz control signal with a conservative 1.0–2.0 ms
pulse range. Actual angle and travel vary by servo; disconnect the linkage or
load during initial testing and stop if the servo binds or buzzes at an end.

### D9 LED diagnostic

To visually check that D9 (PC7) can switch, build the separate diagnostic image:

```sh
cmake --build build --target d9_led_test_nucleo_f446re.elf
```

Connect D9 to an LED in series with a 1 kΩ resistor, then to NUCLEO GND. Use a
red LED for this 3.3 V GPIO test. The diagnostic blinks it with a 500 ms
on/off interval.

This project uses a signed bootloader. Do not flash the raw `.bin` directly at
`0x08020200`: the bootloader expects a signed `.sfu` package, including its
512-byte header, at `0x08020000`. Sign the diagnostic with the same private key
whose public key is provisioned in the bootloader. OpenSSL prompts for the
private-key passphrase; do not put it in a command or copy the private key to
the Pi/NFS:

```sh
python3 tools/secure_firmware.py package \
  --image build/d9_led_test_nucleo_f446re.bin \
  --board STM32F446RE \
  --version-code 10003 \
  --private-key "$HOME/.config/telemetry-bridge/stm32-signing-private.pem" \
  --package build/d9_led_test_nucleo_f446re.sfu

sudo install -m 0644 build/d9_led_test_nucleo_f446re.sfu \
  /srv/nfs/rpi-rootfs/home/stm32-update/
```

On the Pi, verify and flash through the installed wrapper:

```sh
python3 /home/stm32-update/secure_firmware.py verify \
  --package /home/stm32-update/d9_led_test_nucleo_f446re.sfu \
  --public-key /home/stm32-update/stm32-signing-public.pem \
  --board STM32F446RE

systemctl stop gatewayd
/home/stm32-update/flash_signed_stm32.sh \
  /home/stm32-update/d9_led_test_nucleo_f446re.sfu \
  /home/stm32-update/stm32-signing-public.pem
systemctl start gatewayd
```

After the test, restore the servo image by creating and staging a signed
package with a higher version code:

```sh
python3 tools/secure_firmware.py package \
  --image build/sg90_nucleo_f446re.bin \
  --board STM32F446RE \
  --version-code 10004 \
  --private-key "$HOME/.config/telemetry-bridge/stm32-signing-private.pem" \
  --package build/sg90_nucleo_f446re.sfu

sudo install -m 0644 build/sg90_nucleo_f446re.sfu \
  /srv/nfs/rpi-rootfs/home/stm32-update/
```

On the Pi, verify and flash the servo package using the same verifier and
wrapper, then restart `gatewayd`:

```sh
python3 /home/stm32-update/secure_firmware.py verify \
  --package /home/stm32-update/sg90_nucleo_f446re.sfu \
  --public-key /home/stm32-update/stm32-signing-public.pem \
  --board STM32F446RE

systemctl stop gatewayd
/home/stm32-update/flash_signed_stm32.sh \
  /home/stm32-update/sg90_nucleo_f446re.sfu \
  /home/stm32-update/stm32-signing-public.pem
systemctl start gatewayd
```

The diagnostic and servo demo are application images; neither replaces the
bootloader.

### 28BYJ-48 stepper motor test

Plug the 28BYJ-48's 5-wire connector into the ULN2003 driver board. With all
power off, wire the ULN2003 inputs to the NUCLEO-F446RE:

| ULN2003 | NUCLEO-F446RE |
| --- | --- |
| IN1 | D2 / PA10 |
| IN2 | D3 / PB3 |
| IN3 | D4 / PB5 |
| IN4 | D5 / PB4 |
| `+` / VCC | Regulated external 5 V |
| `-` / GND | External supply GND and NUCLEO GND |

The test firmware drives IN1–IN4 in an eight-phase half-step sequence. It
rotates about one output revolution clockwise, de-energizes the coils for one
second, then rotates back and repeats. The 28BYJ-48 gearbox ratio varies
slightly, so the revolution count is approximate. Test with the shaft
unloaded. The ULN2003 board must drive the motor; never connect the motor
windings or the 5 V rail directly to STM32 GPIO. Use a regulated supply with
adequate current and connect its ground to NUCLEO GND. A 4 ms half-step
interval takes about 16 seconds per nominal revolution. The ULN2003 board's
indicator LEDs should sequence while the firmware is stepping; if they do but
the motor does not turn, power off and check the motor connector and coil
wiring. The motor has been confirmed to run from the 5 V rail in this setup.

Build the standalone test image:

```sh
cmake --build build --target stepper_28byj48_nucleo_f446re.elf
```

This project uses the signed bootloader, so do not flash the raw binary. Sign
it with the same private key trusted by the bootloader (OpenSSL prompts for
the passphrase; do not put it in a command or copy the private key to the Pi):

```sh
python3 tools/secure_firmware.py package \
  --image build/stepper_28byj48_nucleo_f446re.bin \
  --board STM32F446RE \
  --version-code 10005 \
  --private-key "$HOME/.config/telemetry-bridge/stm32-signing-private.pem" \
  --package build/stepper_28byj48_nucleo_f446re.sfu

sudo install -m 0644 build/stepper_28byj48_nucleo_f446re.sfu \
  /srv/nfs/rpi-rootfs/home/stm32-update/
```

On the Pi, verify and flash through the installed wrapper:

```sh
python3 /home/stm32-update/secure_firmware.py verify \
  --package /home/stm32-update/stepper_28byj48_nucleo_f446re.sfu \
  --public-key /home/stm32-update/stm32-signing-public.pem \
  --board STM32F446RE

systemctl stop gatewayd
/home/stm32-update/flash_signed_stm32.sh \
  /home/stm32-update/stepper_28byj48_nucleo_f446re.sfu \
  /home/stm32-update/stm32-signing-public.pem
systemctl start gatewayd
```

This test image replaces the currently running application, not the
bootloader. To return to the servo demo, build and sign its application image
with a version code higher than the stepper package (10005 in this example),
then flash it through the same wrapper. For example, package the servo image
with version code 10006:

```sh
python3 tools/secure_firmware.py package \
  --image build/sg90_nucleo_f446re.bin \
  --board STM32F446RE \
  --version-code 10006 \
  --private-key "$HOME/.config/telemetry-bridge/stm32-signing-private.pem" \
  --package build/sg90_nucleo_f446re.sfu

sudo install -m 0644 build/sg90_nucleo_f446re.sfu \
  /srv/nfs/rpi-rootfs/home/stm32-update/
```

Then verify and flash on the Pi:

```sh
python3 /home/stm32-update/secure_firmware.py verify \
  --package /home/stm32-update/sg90_nucleo_f446re.sfu \
  --public-key /home/stm32-update/stm32-signing-public.pem \
  --board STM32F446RE

systemctl stop gatewayd
/home/stm32-update/flash_signed_stm32.sh \
  /home/stm32-update/sg90_nucleo_f446re.sfu \
  /home/stm32-update/stm32-signing-public.pem
systemctl start gatewayd
```

The original F439 target remains available as a compile-verified archive. A
standalone flashable F439 image still needs its exact device startup file and
linker script.

The bundled STMicroelectronics HAL and CMSIS components retain their license notices
in `third_party/stm32cube/Drivers`.
