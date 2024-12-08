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
: "${SQSH_UNPACK:?SQSH_UNPACK is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -all-root -mkfs-time 0"

WORK_DIR="$BUILD_DIR/unpack-empty_file"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

mkdir -p root unpack

touch root/empty_file

# shellcheck disable=SC2086
$MKSQUASHFS "root" "empty_file.squashfs" $MKSQUASHFS_OPTS

$SQSH_UNPACK -Ve "$PWD/empty_file.squashfs" / "$PWD/unpack"
