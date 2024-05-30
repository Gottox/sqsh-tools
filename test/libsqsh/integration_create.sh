#!/bin/sh -e

in=$1
out=$2
tmp=$3

mkdir -p "$tmp"
echo a > "$tmp/a"
# OpenBSD uses floating point for seq, so it may no precise about the number
# of lines.  Use head to make sure we have the right number of lines.
seq 1 1050000 | head -1050000 | tr -cd "\n" | tr '\n' b > "$tmp/b"
mkdir -p "$tmp/large_dir"
seq 1 1050 | sed "s#.*#$tmp/large_dir/&#" | xargs touch
ln -s .. "$tmp/large_dir/link"
[ -e "$out" ] && rm "$out"
$MKSQUASHFS "$tmp" "$out.tmp" \
	-p '"large_dir" x user.force_extended=true' \
	-p '"a" x user.foo=1234567891234567891234567890001234567890' \
	-p '"b" x user.bar=1234567891234567891234567890001234567890' \
	-nopad \
	-force-uid 2020 \
	-force-gid 202020 \
	-Xcompression-level 1 \
	-always-use-fragments \
	-quiet -no-progress

dd if=/dev/zero of="$out" bs=1 count=0 seek=1010 2>/dev/null
cat "$out.tmp" >> "$out"
rm "$out.tmp"
