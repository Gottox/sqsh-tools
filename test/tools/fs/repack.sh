#!/bin/sh -ex

######################################################################
# @author       Enno Boland (mail@eboland.de)
# @file         repack.sh
# @created      Friday Mar 17, 2023 15:11:09 CET
#
# @description  This script creates a squashfs image, mounts it, and
#               repacks it from the mounted path.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSHFS:?SQSHFS is not set}"

unmount() {
	for _ in 1 2 3 4 5; do
		if fusermount -uz "$1" || umount "$1"; then
			return
		fi
		sleep 0.5
	done
	rmdir "$1"
}

MKSQUASHFS_OPTS="-no-xattrs -noappend -mkfs-time 0"

SQSHFS_IMPL="$(basename "$SQSHFS")"

WORK_DIR="$BUILD_DIR/$SQSHFS_IMPL/fs-repack"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# shellcheck disable=SC2086
$MKSQUASHFS "$SOURCE_ROOT/libsqsh" "$PWD/original.squashfs" $MKSQUASHFS_OPTS

MOUNT_POINT="$(mktemp -d)"
$SQSHFS "$PWD/original.squashfs" "$MOUNT_POINT"

# shellcheck disable=SC2086
$MKSQUASHFS "$MOUNT_POINT" "$PWD/repacked.squashfs" $MKSQUASHFS_OPTS

unmount "$MOUNT_POINT"

exec cmp "$PWD/original.squashfs" "$PWD/repacked.squashfs"
