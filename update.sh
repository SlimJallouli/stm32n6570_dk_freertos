#!/bin/bash

# Define source and destination directories
HOME=~
mbedTLS_VERSON="3.1.1"
mbedTLS_library="$HOME/STM32Cube/Repository/Packs/ARM/mbedTLS/$mbedTLS_VERSON/library/"
mbedTLS_include="$HOME/STM32Cube/Repository/Packs/ARM/mbedTLS/$mbedTLS_VERSON/include/"
mbedTLS_destination="./Middlewares/Third_Party/ARM_Security/"

# Create the destination directory if it doesn't exist
if [ ! -d "$mbedTLS_destination" ]; then
    mkdir -p "$mbedTLS_destination"
fi

echo "Home : " $HOME

# Copy the contents from mbedTLS_library to mbedTLS_destination
cp -r "$mbedTLS_library" "$mbedTLS_destination"
echo "Content copied from $mbedTLS_library to $mbedTLS_destination"

cp -r "$mbedTLS_include" "$mbedTLS_destination"
echo "Content copied from $mbedTLS_include to $mbedTLS_destination"

FILE_PATH="./Core/Src/sysmem.c"
echo "Deleting $FILE_PATH"
rm -f $FILE_PATH

FILE_PATH="./Middlewares/Third_Party/lwIP_Network_lwIP/ports/freertos/sys_arch.c"
echo "Deleting $FILE_PATH"
rm -f $FILE_PATH

FOLDER_PATH="./Middlewares/Third_Party/lwIP_Network_lwIP/ports/freertos/include/arch"
echo "Deleting $FOLDER_PATH"
rm -rf $FOLDER_PATH

FOLDER_PATH="./Middlewares/Third_Party/lwIP_Network_lwIP/rte"
echo "Deleting $FOLDER_PATH"
rm -rf $FOLDER_PATH

FILE_PATH="./Src/sysmem.c"
echo "Deleting $FILE_PATH"
rm -f $FILE_PATH

FOLDER_PATH="./Libraries/littlefs/runners"
echo "Deleting $FOLDER_PATH"
rm -rf $FOLDER_PATH

FOLDER_PATH="./Libraries/corePKCS11/test"
echo "Deleting $FOLDER_PATH"
rm -rf $FOLDER_PATH
