#!/bin/sh -e

in=$1
out=$2
tmp=$3

mkdir -p "$tmp"
echo a > "$tmp/a"
seq 1 1050000 | tr -cd "\n" | tr '\n' b > "$tmp/b"
mkdir -p "$tmp/large_dir"
seq 1 1050 | sed "s#.*#$tmp/large_dir/&#" | xargs touch
[ -e "$out" ] && rm "$out"
$MKSQUASHFS "$tmp" "$out" \
	-p '"a" x user.foo=1234567891234567891234567890001234567890' \
	-p '"b" x user.bar=1234567891234567891234567890001234567890' \
	-p '"large_dir" x user.force_extended=true' \
	-nopad \
	-force-uid 2020 \
	-force-gid 202020 \
	-Xcompression-level 1 \
	-always-use-fragments \
	-quiet -no-progress