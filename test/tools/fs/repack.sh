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
: "${UNSHARE:?UNSHARE is not set}"
: "${FUSERMOUNT:?FUSERMOUNT is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -mkfs-time 0"

SQSHFS_IMPL="$(basename "$SQSHFS")"

# unshares the mount namespace, so that sqshfs will be terminated
# when this script exits
if [ -z "$INTERNAL_UNSHARED" ]; then
	export INTERNAL_UNSHARED=1
	exec "$UNSHARE" -rm "$0" "$@"
fi

WORK_DIR="$BUILD_DIR/$SQSHFS_IMPL/fs-repack"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# shellcheck disable=SC2086
$MKSQUASHFS "$SOURCE_ROOT/.git" "$PWD/original.squashfs" $MKSQUASHFS_OPTS

mkdir -p "mnt"

$SQSHFS "$PWD/original.squashfs" "$PWD/mnt"

# shellcheck disable=SC2086
$MKSQUASHFS "$PWD/mnt" "$PWD/repacked.squashfs" $MKSQUASHFS_OPTS

$FUSERMOUNT -u "mnt"

exec cmp "$PWD/original.squashfs" "$PWD/repacked.squashfs"
