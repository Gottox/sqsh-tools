#!/usr/bin/env sh

######################################################################
# @author       Enno Boland (mail@eboland.de)
# @file         f2h.sh
# @created      Sunday Jun 11, 2023 23:15:34 CEST
#
# @description  converts a binary file to a header file
######################################################################


var_name=$1
in=$2
out=$3

printf "%s\n" \
	"#include <stddef.h>" \
	"unsigned char ${var_name}[] = {"  > "$out"
hexdump -v -e '"0x" 1/1 "%02X" ", "' "$in" >> "$out"
printf "%s\n" \
	"};" \
	"size_t ${var_name}_size = sizeof($var_name);" >> "$out"
