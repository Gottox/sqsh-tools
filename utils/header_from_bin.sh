#!/bin/sh -e
in=$1
out=$2
tmp=$3

mkdir -p "$tmp"
printf "%s\n" "#include <stdint.h>" "static const uint8_t squash_image[] = {" > "$tmp/header.h"
od $in -t u1 -v -A n | tr ' ' '\n' | grep -v '^$' | paste -sd, >> "$tmp/header.h"
echo "};" >> "$tmp/header.h"

mv "$tmp/header.h" "$out"
