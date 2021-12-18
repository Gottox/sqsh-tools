#!/bin/sh -e

in=$1
out=$2
tmp=$3

mkdir -p "$tmp"
echo a > "$tmp/a"
seq 1 1050000 | tr -cd "\n" | tr '\n' b > "$tmp/b"
$SETFATTR -n user.foo -v 1234567891234567891234567890001234567890 "$tmp/a"
$SETFATTR -n user.bar -v 1234567891234567891234567890001234567890 "$tmp/b"
[ -e "$out" ] && rm "$out"
$MKSQUASHFS "$tmp" "$out" \
	-noI -noId -noD -noF -noX \
	-nopad \
	-force-uid 2020 \
	-force-gid 202020 \
	-Xcompression-level 1 \
	-always-use-fragments \
	-quiet
