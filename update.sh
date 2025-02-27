#!/bin/bash

# Define source and destination directories
HOME=~
FreeRTOS_VERSON="11.1.0"

FreeRTOS_CONFIG_FILE="FreeRTOSConfig.h"

FreeRTOS_SRC_Folder="$HOME/STM32Cube/Repository/Packs/ARM/CMSIS-FreeRTOS/$FreeRTOS_VERSON/Source"
FreeRTOS_DST_Folder="./Middlewares/Third_Party/ARM_RTOS_FreeRTOS"

# Create the destination directory if it doesn't exist
if [ ! -d "$FreeRTOS_DST_Folder" ]; then
    mkdir -p "$FreeRTOS_DST_Folder"
fi

echo "Home : " $HOME

# Copy the contents from mbedTLS_source to mbedTLS_destination
FILE_TO_COPY="FreeRTOSConfig.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder/examples/template_configuration"
DST_FOLDER="$FreeRTOS_DST_Folder/Config/ARMCM/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="croutine.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="list.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="queue.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="stream_buffer.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="tasks.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="timers.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="event_groups.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="heap_4.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder/portable/MemMang"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/portable/MemMang/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/CMakeLists.txt"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/FreeRTOS.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/StackMacros.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/atomic.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/croutine.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/deprecated_definitions.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/event_groups.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/message_buffer.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/mpu_prototypes.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/mpu_syscall_numbers.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/mpu_wrappers.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/newlib-freertos.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/picolibc-freertos.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/portable.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/projdefs.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/queue.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/semphr.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/stack_macros.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/stdint.readme"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/stream_buffer.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/task.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/timers.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="include/list.h"
SRC_FOLDER="$FreeRTOS_SRC_Folder"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/include"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"

rm -rf "$FreeRTOS_DST_Folder/Source/portable/GCC/ARM_CM33_NTZ"

SRC_FOLDER="$FreeRTOS_SRC_Folder/portable/GCC/ARM_CM55_NTZ"
DST_FOLDER="$FreeRTOS_DST_Folder/Source/portable/GCC/"
cp -r "$SRC_FOLDER" "$DST_FOLDER"
echo "Content copied from $SRC_FOLDER to $DST_FOLDER"

FILE_TO_COPY="port.c"
SRC_FOLDER="$FreeRTOS_SRC_Folder/portable/GCC/ARM_CM55_NTZ/non_secure"
DST_FOLDER="Middlewares/FreeRTOS/RTOS/Core/Cortex-M/"
cp "$SRC_FOLDER/$FILE_TO_COPY" "$DST_FOLDER"
echo "Copied $FILE_TO_COPY from $SRC_FOLDER to $DST_FOLDER"