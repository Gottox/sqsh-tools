#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : unpack-test.sh
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : This script creates a squashfs image, mounts it, and
#                repacks it from the mounted path.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSH_UNPACK:?SQSH_UNPACK is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -all-root -mkfs-time 0"

cd "$SOURCE_ROOT"

tmpdir="$BUILD_DIR/unpack-dir"
mkdir -p "$tmpdir"

ORIGINAL_IMAGE="$tmpdir/original.img"
REPACKED_IMAGE="$tmpdir/unpack.img"

# shellcheck disable=SC2086
$MKSQUASHFS .git "$ORIGINAL_IMAGE" $MKSQUASHFS_OPTS

UNPACK_DIR="$tmpdir/unpack"

mkdir -p "$UNPACK_DIR"

$SQSH_UNPACK "$ORIGINAL_IMAGE" / "$UNPACK_DIR"

# shellcheck disable=SC2086
$MKSQUASHFS "$UNPACK_DIR" "$REPACKED_IMAGE" $MKSQUASHFS_OPTS

exec cmp "$ORIGINAL_IMAGE" "$REPACKED_IMAGE"
