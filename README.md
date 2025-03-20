# STM32N6570 DK FreeRTOS

This repository contains the FreeRTOS-based project for the [STM32N6570-DK Discovery kit with STM32N657X0 MCU](https://www.st.com/en/evaluation-tools/stm32n6570-dk.html). This project helps you get started with STM32N6 and FreeRTOS.

## Getting Started

To clone the repository and initialize the submodules, use the following commands:

```bash
git clone https://github.com/SlimJallouli/stm32n6570_dk_freertos.git
git submodule update --init --recursive
```

## CMSIS packs
* [ARM.CMSIS-FreeRTOS.11.1.0.pack](https://keilpack.azureedge.net/pack/ARM.CMSIS-FreeRTOS.11.1.0.pack)

## Tools
* [STM32CubeIDE](https://www.st.com/stm32cubeide) 1.18.0 or higher
* [STM32CubeMX](http://www.st.com/stm32cubemx) 6.13.0 or higher
* [STM32CubeProgrammer](http://www.st.com/stm32cubeprog) 2.28.0 or higher
* [GitBash](https://git-scm.com/downloads)

## Build the project
* Dowload [ARM.CMSIS-FreeRTOS.11.1.0.pack](https://keilpack.azureedge.net/pack/ARM.CMSIS-FreeRTOS.11.1.0.pack) and install it using STM32CubeMX.
* Open the 'stm32n6570_dk_freertos.ioc' file with [STM32CubeMX](http://www.st.com/stm32cubemx)
* Open it with [STM32CubeIDE](https://www.st.com/stm32cubeide) build the project FSBL and Appli projects .

## Flash the project
* Make sure the board boot switches are set to Dev mode.
* Use the flash.sh file:

```bash
bash flash.sh
```

## Debug
* Start the debug session using STM32CubeIDE.
* Use a serial terminal to monitor the debug output.
