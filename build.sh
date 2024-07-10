#!/bin/bash

# Function to display usage information
usage() {
    echo "Usage: $0 <kernel_dir_path> <cross_compile_toolchain>"
    echo "Example: $0 /path/to/kernel/sources mipsel-linux-"
    exit 1
}

# Check if the correct number of arguments is provided
if [[ $# -ne 2 ]]; then
    usage
fi

# Assign arguments to variables
KERNEL_DIR="$1"
CROSS_COMPILE_TOOLCHAIN="$2"

# Check if kernel directory is specified
if [[ -z "$KERNEL_DIR" ]]; then
    echo "Error: Kernel directory path not specified."
    usage
fi

# Check if cross-compile toolchain is specified
if [[ -z "$CROSS_COMPILE_TOOLCHAIN" ]]; then
    echo "Error: Cross-compile toolchain not specified."
    usage
fi

# Run make command
/usr/bin/make -j$(nproc) \
-C "$KERNEL_DIR" \
ARCH=mips \
CROSS_COMPILE="$CROSS_COMPILE_TOOLCHAIN" \
KDIR="$KERNEL_DIR" \
PWD=$(pwd) \
M=$(pwd)/. modules
