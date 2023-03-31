#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : tail.sh
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : This script creates a squashfs image, mounts it, and
#                repacks it from the mounted path.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${READ_CHUNK:?READ_CHUNK is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -mkfs-time 0 -b 4096"

WORK_DIR="$BUILD_DIR/read-chunk-tail"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

OFFSET=262150

seq 1 "$OFFSET" | head -c "$OFFSET" > "$PWD/file"

# shellcheck disable=SC2086
$MKSQUASHFS "$PWD/file" "$PWD/original.squashfs" $MKSQUASHFS_OPTS

tail -c 6 "$PWD/file" > tail.sqsh-cat
$READ_CHUNK "$PWD/original.squashfs" file 262144 6 > tail.read-chunk

exec cmp "$PWD/tail.sqsh-cat" "$PWD/tail.read-chunk"
