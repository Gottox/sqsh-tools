#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : squashfs-test.sh
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : This script creates a squashfs image, mounts it, and
#                repacks it from the mounted path.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSHFS:?SQSHFS is not set}"
: "${UNSHARE:?UNSHARE is not set}"
: "${FUSERMOUNT:?FUSERMOUNT is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -mkfs-time 0"

# unshares the mount namespace, so that sqshfs will be terminated
# when this script exits
if [ -z "$INTERNAL_UNSHARED" ]; then
	export INTERNAL_UNSHARED=1
	exec "$UNSHARE" -rm "$0" "$@"
fi

cd "$SOURCE_ROOT"

tmpdir="$BUILD_DIR/sqshfs-test"
mkdir -p "$tmpdir"

ORIGINAL_IMAGE="$tmpdir/original.img"
REPACKED_IMAGE="$tmpdir/repacked.img"

# shellcheck disable=SC2086
$MKSQUASHFS .git "$ORIGINAL_IMAGE" $MKSQUASHFS_OPTS

MOUNT_POINT="$tmpdir/mnt"

mkdir -p "$MOUNT_POINT"

$SQSHFS "$ORIGINAL_IMAGE" "$MOUNT_POINT"

# shellcheck disable=SC2086
$MKSQUASHFS "$MOUNT_POINT" "$REPACKED_IMAGE" $MKSQUASHFS_OPTS

$FUSERMOUNT -u "$MOUNT_POINT"

exec cmp "$ORIGINAL_IMAGE" "$REPACKED_IMAGE"
