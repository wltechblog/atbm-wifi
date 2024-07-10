#!/bin/bash

# Function to display usage
usage() {
	echo "Usage: $0 [-f <header_file_path>] <header_file_path> <output_bin_file_path>"
	echo "  -f    Display the firmware version from the header file"
	exit 1
}

# Function to display firmware version
display_version() {
	header_file_path=$1
	
	# Extract the version value (bytes 5 to 8 in little-endian order)
	version_bytes=$(grep -o '0x[0-9a-fA-F]\{2\}' "$header_file_path" | sed -n '5,8p' | tac | tr -d '\n' | sed 's/0x//g')
	
	# Convert the version from hex to decimal
	version_decimal=$((16#$version_bytes))
	
	echo "Firmware version: $version_decimal"
}

# Parse the options
while getopts ":f:" opt; do
	case $opt in
		f)
			display_version_flag=true
			header_file_path=$OPTARG
			;;
		*)
			usage
			;;
	esac
done
shift $((OPTIND -1))

# Check if the -f flag is set and process accordingly
if [ "$display_version_flag" = true ]; then
	if [ -z "$header_file_path" ]; then
		usage
	fi

	if [ ! -f "$header_file_path" ]; then
		echo "Header file not found: $header_file_path"
		exit 1
	fi
	
	display_version "$header_file_path"
	exit 0
fi

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
	usage
fi

header_file_path=$1
output_bin_file_path=$2

# Check if the header file exists
if [ ! -f "$header_file_path" ]; then
	echo "Header file not found: $header_file_path"
	exit 1
fi

# Extract hexadecimal values and write to binary file
grep -o '0x[0-9a-fA-F]\{2\}' "$header_file_path" | xxd -r -p > "$output_bin_file_path"

echo "Conversion complete: $output_bin_file_path"
