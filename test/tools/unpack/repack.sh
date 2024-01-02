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
: "${SQSH_UNPACK:?SQSH_UNPACK is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -all-root -mkfs-time 0"

WORK_DIR="$BUILD_DIR/unpack-repack"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# shellcheck disable=SC2086
$MKSQUASHFS "$SOURCE_ROOT/.git" "original.squashfs" $MKSQUASHFS_OPTS

mkdir -p "unpack"

$SQSH_UNPACK "$PWD/original.squashfs" / "$PWD/unpack"

# shellcheck disable=SC2086
$MKSQUASHFS "unpack" "repacked.squashfs" $MKSQUASHFS_OPTS

rm -rf "unpack"

exec cmp "original.squashfs" "repacked.squashfs"
