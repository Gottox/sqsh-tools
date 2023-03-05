#!/bin/sh

set -ex

: "${SQSH_LS:?SQSH_LS not set}"
: "${SQSH_CAT:?SQSH_CAT not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT not set}"

cd "$SOURCE_ROOT"

tmpdir=$(mktemp -d)

PACKED="$tmpdir/squashfs-packed.img"
UNPACKED="$tmpdir/squashfs-unpacked.img"

mksquashfs .git "$PACKED" -noappend -keep-as-directory
mksquashfs .git "$UNPACKED" -noappend -keep-as-directory -noI -noId -noD -noF -noX

$SQSH_LS -r "$PACKED" . > "$tmpdir"/packed.list
$SQSH_LS -r "$UNPACKED" . > "$tmpdir"/unpacked.list

cmp "$tmpdir"/packed.list "$tmpdir"/unpacked.list

while read -r; do
	if ! [ -f "$REPLY" ] || [ -h "$REPLY" ]; then
		continue;
	fi
	$SQSH_CAT "$PACKED" "$REPLY" | cmp - "$REPLY"
	$SQSH_CAT "$UNPACKED" "$REPLY" | cmp - "$REPLY"
done < "$tmpdir"/packed.list

rm -r "$tmpdir"
