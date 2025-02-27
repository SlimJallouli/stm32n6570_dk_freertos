#!/bin/bash

HOME=~
COMPILER="GCC"
CORE="ARM_CM55_NTZ"
FreeRTOS_VERSION="11.1.0"

# Define source and destination directories
FreeRTOS_SRC_Folder="$HOME/STM32Cube/Repository/Packs/ARM/CMSIS-FreeRTOS/$FreeRTOS_VERSION/Source"
FreeRTOS_DST_Folder="./Middlewares/Third_Party/ARM_RTOS_FreeRTOS"

# Create the destination directory if it doesn't exist
mkdir -p "$FreeRTOS_DST_Folder"

echo "Home: $HOME"

# Function to copy files and echo progress
copy_file() {
    local src="$1"
    local dst="$2"
    cp "$src" "$dst"
    echo "Copied $(basename "$src") from $src to $dst"
}

# List of files to copy
files_to_copy=(
    "examples/template_configuration/FreeRTOSConfig.h:Config/ARMCM/"
    "croutine.c:Source/"
    "list.c:Source/"
    "queue.c:Source/"
    "stream_buffer.c:Source/"
    "tasks.c:Source/"
    "timers.c:Source/"
    "event_groups.c:Source/"
    "portable/MemMang/heap_4.c:Source/portable/MemMang/"
    "include/CMakeLists.txt:Source/include"
    "include/FreeRTOS.h:Source/include"
    "include/StackMacros.h:Source/include"
    "include/atomic.h:Source/include"
    "include/croutine.h:Source/include"
    "include/deprecated_definitions.h:Source/include"
    "include/event_groups.h:Source/include"
    "include/message_buffer.h:Source/include"
    "include/mpu_prototypes.h:Source/include"
    "include/mpu_syscall_numbers.h:Source/include"
    "include/mpu_wrappers.h:Source/include"
    "include/newlib-freertos.h:Source/include"
    "include/picolibc-freertos.h:Source/include"
    "include/portable.h:Source/include"
    "include/projdefs.h:Source/include"
    "include/queue.h:Source/include"
    "include/semphr.h:Source/include"
    "include/stack_macros.h:Source/include"
    "include/stdint.readme:Source/include"
    "include/stream_buffer.h:Source/include"
    "include/task.h:Source/include"
    "include/timers.h:Source/include"
    "include/list.h:Source/include"
)

# Copy each file
for entry in "${files_to_copy[@]}"; do
    IFS=":" read -r src_path dst_path <<< "$entry"
    copy_file "$FreeRTOS_SRC_Folder/$src_path" "$FreeRTOS_DST_Folder/$dst_path"
done

# Remove old portable directory and copy the new one
rm -rf "$FreeRTOS_DST_Folder/Source/portable/$COMPILER/ARM_CM33_NTZ"
copy_file "$FreeRTOS_SRC_Folder/portable/$COMPILER/$CORE" "$FreeRTOS_DST_Folder/Source/portable/$COMPILER/"

# Copy port.c specifically to a different destination
copy_file "$FreeRTOS_SRC_Folder/portable/$COMPILER/$CORE/non_secure/port.c" "Middlewares/FreeRTOS/RTOS/Core/Cortex-M/"

echo "All files copied successfully."
