# PIC16F877A Microwave Oven Simulation

A menu-driven microwave oven controller simulation written in embedded C for the **Microchip PIC16F877A**. The project demonstrates keypad scanning, LCD control, Timer2 interrupts, cooking-mode state handling, and simulated fan, buzzer, and door-status behavior. It is an educational microcontroller simulation; it does not control a real microwave magnetron or other high-voltage appliance hardware.

## Features

- Four-line character LCD startup screen and mode menu.
- 4×3 matrix-keypad input for selecting modes and entering settings.
- Micro, Grill, and Convection options.
- Cooking-time entry, countdown display, start/resume, pause, and stop controls.
- Convection temperature entry with a maximum accepted value of 250 (as implemented in the source).
- Timer2 interrupt-based countdown logic.
- Simulated fan, buzzer, and door-open indication.

## Hardware and software

| Item | Project setting / implementation |
| --- | --- |
| MCU | PIC16F877A |
| IDE | MPLAB X (project files are under `microwave_oven.X/nbproject`) |
| Compiler | XC8 3.10, as recorded in `configurations.xml` |
| LCD | 4-bit interface, control/data pins described in `clcd.h` |
| Keypad | 4 rows × 3 columns, pin mappings described in `matrix_keypad.h` |
| Timer | Timer2, configured in `timers.c`; countdown serviced in `isr.c` |

The source defines `_XTAL_FREQ` as 20 MHz for LCD delay macros. Confirm the oscillator used in your simulator or hardware matches this value. The exact LCD wiring, board setup, and any simulator-specific configuration should also be checked against your circuit; not all electrical configuration is documented in the source.

## Repository layout

```text
microwave_oven.X/
├── main.c                 Application flow and cooking-mode state handling
├── main.h                 State constants, pin aliases, and application declarations
├── clcd.c / clcd.h        4-bit character LCD driver
├── matrix_keypad.c/.h     Matrix keypad initialization and scanning
├── timers.c / timers.h    Timer2 initialization
├── isr.c                  Timer2 interrupt service routine
├── Makefile               MPLAB X generated build entry point
└── nbproject/             MPLAB X project and device/toolchain configuration
```

`build/` and `dist/` are generated output directories and are not required to understand or build the source project; MPLAB X regenerates them during a build.

## User controls (as implemented)

| Key | Action |
| --- | --- |
| `1` | Select Micro mode |
| `2` | Select Grill mode |
| `3` | Select Convection mode |
| `4` | Start the default 30-second cycle from the menu; add 30 seconds on the time screen; resume from pause |
| `5` | Pause |
| `6` | Stop and return to the menu |
| `*` | Clear the current setting field during time/temperature entry |
| `#` | Confirm the setting and continue/start the countdown |
| `0`–`9` | Enter time or temperature digits |

The key mapping describes the current source behavior. In particular, keypad event handling and the time-entry field order should be checked if the observed interaction differs from the intended `MM:SS` flow.

## Application flow

1. `main()` initializes the LCD, keypad, Timer2, fan and buzzer pins, enables peripheral/global interrupts, and displays a startup banner.
2. The main loop reads keypad input and selects a cooking mode or handles start, pause, resume, and stop requests.
3. Micro and Grill use `set_time()` to collect a cooking duration. Convection first calls `set_temp()` and, after its preheat phase, proceeds to time entry.
4. `time_dis()` displays the remaining duration and controls. Timer2 interrupts periodically update `min` and `sec` in `isr.c`.
5. The door check pauses the timer and fan and displays a warning while the door is open. When the countdown reaches zero, the program stops the fan, sounds the buzzer briefly, and returns to the menu.

## Open and build in MPLAB X

1. Install MPLAB X and the XC8 compiler. The project metadata currently specifies XC8 3.10 and the PIC16F877A device pack; use compatible versions or update the project configuration as needed.
2. In MPLAB X, choose **File → Open Project** and select `microwave_oven.X` (the folder containing `nbproject/`).
3. Confirm the selected device is **PIC16F877A**, and review the oscillator, configuration bits, and simulator/tool settings for your setup.
4. Select **Clean and Build Project**. MPLAB X should generate the output under `microwave_oven.X/dist/`.
5. To simulate, select an available simulator/debug tool in MPLAB X, load the generated program, and connect virtual LCD/keypad/peripheral components using the mappings in the source and your circuit design.

The repository configuration records the device and compiler version, but it does not by itself provide a complete simulator schematic or wiring guide. Add your circuit/simulator file and pinout notes if you want others to reproduce the simulation exactly.

## Timer and implementation notes

`timers.c` sets Timer2 prescaling and `PR2`; `isr.c` then counts **1,250 Timer2 interrupts** before decrementing the displayed time by one second. The resulting interval depends on the actual oscillator frequency and Timer2 configuration. Measure or calculate the interval for your simulator/device before relying on the countdown for accurate timing.

The current source is suitable as a learning example, but some behaviors merit review before treating it as a robust controller:

- Verify the keypad read mode: `main.c` passes `STATE`, while `matrix_keypad.c` uses `LEVEL` for one-event-per-press behavior.
- Verify time digit ordering and validate seconds/minutes ranges before accepting the entry.
- Review door handling so it does not automatically restart a paused or stopped cycle when the door closes.
- The convection preheat loop is blocking, so keypad and door input are not serviced during that wait.
- On an 8-bit PIC, shared multi-byte countdown values modified by an ISR may need `volatile` declarations and protected/atomic reads and writes.
- Confirm the door pin direction and digital/analog configuration against the actual PIC setup and circuit.

These are source-review observations, not a claim that the simulation has been hardware-validated.

## Source files

The main application and drivers are in `microwave_oven.X/`. `Makefile` and `nbproject/` contain MPLAB X build metadata. Generated build outputs can be recreated and are generally not necessary to commit.

## License

No license is currently specified. Add a license file before redistributing or reusing this project so the intended usage terms are clear.
