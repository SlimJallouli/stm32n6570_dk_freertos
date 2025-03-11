APP_NAME="stm32n6570_dk_freertos"
FSBL_BIN_FILE="./FSBL/Debug/stm32n6570_dk_freertos_FSBL.bin"
APP_BIN_FILE="./Appli/Debug/stm32n6570_dk_freertos_Appli.bin"

rm -f *.bin

PROGRAMMER="C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin//STM32_Programmer_CLI.exe"
SIGNING_TOOL="C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_SigningTool_CLI.exe"
EXTERNAL_LOADER="C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr"
 
FLASH_NOR_ADDRESS=0x70000000
FLASH_APP_ADDRESS=0x70100000
#  Sign the binay
"$SIGNING_TOOL" -bin $FSBL_BIN_FILE -nk -of 0x80000000 -t fsbl -o FSBL-trusted.bin -hv 2.3 -dump FSBL-trusted.bin
"$SIGNING_TOOL" -bin $APP_BIN_FILE -nk -of 0x80000000 -t fsbl -o Project-trusted.bin -hv 2.3 -dump Project-trusted.bin

# Flash the device
"$PROGRAMMER" -c port=SWD mode=HOTPLUG ap=1 -w FSBL-trusted.bin $FLASH_NOR_ADDRESS -el "$EXTERNAL_LOADER"
"$PROGRAMMER" -c port=SWD mode=HOTPLUG ap=1 -w Project-trusted.bin $FLASH_APP_ADDRESS -el "$EXTERNAL_LOADER"
