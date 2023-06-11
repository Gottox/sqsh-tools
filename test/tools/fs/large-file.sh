#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : repack.sh
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : This script creates a squashfs image, mounts it, and
#                repacks it from the mounted path.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSHFS:?SQSHFS is not set}"

unmount() {
	fusermount -u "$1" || umount "$1"
}

MKSQUASHFS_OPTS="-no-xattrs -noappend -mkfs-time 0 -b 4096"

SQSHFS_IMPL="$(basename "$SQSHFS")"

WORK_DIR="$BUILD_DIR/$SQSHFS_IMPL/fs-large-file"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

OFFSET=262150

seq 1 "$OFFSET" | head -c "$OFFSET" > "$PWD/file.orig"

# shellcheck disable=SC2086
$MKSQUASHFS "$PWD/file.orig" "$PWD/original.squashfs" $MKSQUASHFS_OPTS

mkdir -p "$PWD/mnt"

$SQSHFS "$PWD/original.squashfs" "$PWD/mnt"

cat "$PWD/mnt/file.orig" > "$PWD/file.extracted"

unmount "$PWD/mnt"

exec cmp "$PWD/file.orig" "$PWD/file.extracted"
