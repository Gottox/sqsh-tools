#!/bin/sh -e

in=$1
out=$2
tmp=$3

mkdir -p "$tmp/empty"

# Check if the directory is actually empty
[ -z "$(find "$tmp/empty" -mindepth 1)" ]

cat > $tmp/pf <<EOF
"a" F 0 777 2020 202020 echo a
"b" F 0 777 2020 202020 seq 1 1050000 | head -1050000 | tr -cd "\n" | tr '\n' b
"large_dir" D 0 777 2020 202020
"large_dir/link" s 777 2020 202020 ..
EOF
if [ `uname` != "OpenBSD" ]; then
	cat >> $tmp/pf <<EOF
"large_dir" x user.force_extended=true
"a" x user.foo=1234567891234567891234567890001234567890
"b" x user.bar=1234567891234567891234567890001234567890
EOF
fi

for i in $(seq 1 1000); do
	printf '"large_dir/%i" I 0 777 2020 202020 f\n' "$i" >> $tmp/pf
done

[ -e "$tmp/image" ] && rm "$tmp/image"
$MKSQUASHFS "$tmp/empty" "$tmp/image" \
	-pf "$tmp/pf" \
	-noappend \
	-nopad \
	-force-uid 2020 \
	-force-gid 202020 \
	-Xcompression-level 1 \
	-always-use-fragments \
	-quiet -no-progress

dd if=/dev/zero of="$out" bs=1 count=0 seek=1010 2>/dev/null
cat "$tmp/image" >> "$out"
