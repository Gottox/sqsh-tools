#!/bin/sh -ex

######################################################################
# @author       Enno Boland (mail@eboland.de)
# @file         utc.sh
# @created      Monday Nov 20, 2023 18:24:05 CET

#
# @description  This script creates a squashfs image, mounts it, and
#               repacks it from the mounted path.
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSH_LS:?SQSH_UNPACK is not set}"

MKSQUASHFS_OPTS="-no-xattrs -noappend -all-root -mkfs-time 0"

WORK_DIR="$BUILD_DIR/utc"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

mkdir -p "$PWD/empty"

for i in $(seq 1 3641); do
  echo "dir$i d 777 0 0";
  echo "dir$i/file c 776 0 0 100 1";
done > "$PWD/utc.pseudo";

# shellcheck disable=SC2086
$MKSQUASHFS "$PWD/empty" "$PWD/large_tree.squashfs" -pf "$PWD/utc.pseudo" \
	$MKSQUASHFS_OPTS
$SQSH_LS -r -l --utc "$PWD/large_tree.squashfs"
$SQSH_LS -r "$PWD/large_tree.squashfs"
