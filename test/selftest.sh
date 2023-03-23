#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : repacktest
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : This script creates a squashfs image, lists all
#                files and cats them.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSH_CAT:?SQSH_CAT is not set}"
: "${SQSH_LS:?SQSH_LS is not set}"
: "${SQSH_LZO_HELPER_PATH:?SQSH_LZO_HELPER_PATH is not set}"

cd "$SOURCE_ROOT"


tmpdir="$BUILD_DIR/repacktest"
mkdir -p "$tmpdir"

PACKED="$tmpdir/squashfs-packed.img"
UNPACKED="$tmpdir/squashfs-unpacked.img"

$MKSQUASHFS .git "$PACKED" -noappend -keep-as-directory -b 4096
$MKSQUASHFS .git "$UNPACKED" -noappend -keep-as-directory -b 4096 -noI -noId -noD -noF -noX

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
