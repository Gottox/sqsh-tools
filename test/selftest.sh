#!/bin/sh

BUILD_DIR=build_dir
set -ex
cd "$(dirname "$0")"
until [ -d .git ]; do cd ..; done

tmpdir=$(mktemp -d)

PACKED="$tmpdir/squashfs-packed.img"
UNPACKED="$tmpdir/squashfs-unpacked.img"

mksquashfs .git "$PACKED" -noappend -keep-as-directory
mksquashfs .git "$UNPACKED" -noappend -keep-as-directory -noI -noId -noD -noF -noX

$BUILD_DIR/sqsh-ls -r "$PACKED" . > "$tmpdir"/packed.list
$BUILD_DIR/sqsh-ls -r "$UNPACKED" . > "$tmpdir"/unpacked.list

cmp "$tmpdir"/packed.list "$tmpdir"/unpacked.list

while read -r; do
	[ -f "$REPLY" ] && ! [ -h "$REPLY" ] || continue;
	$BUILD_DIR/sqsh-cat "$PACKED" "$REPLY" | cmp - "$REPLY"
	$BUILD_DIR/sqsh-cat "$UNPACKED" "$REPLY" | cmp - "$REPLY"
done < "$tmpdir"/packed.list

rm -r "$tmpdir"
