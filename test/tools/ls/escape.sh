#!/bin/sh -ex

######################################################################
# @author       Enno Boland (mail@eboland.de)
# @file         escape.sh
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

WORK_DIR="$BUILD_DIR/escape"

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

mkdir -p "$PWD/empty"

printf "dir\33 d 777 0 0\n"  > "$PWD/escape.pseudo";

# shellcheck disable=SC2086
$MKSQUASHFS "$PWD/empty" "$PWD/escape.squashfs" -pf "$PWD/escape.pseudo" \
	$MKSQUASHFS_OPTS
result="$($SQSH_LS -r --escape "$PWD/escape.squashfs")"
[ "$result" = "$(printf "%s" "/dir\e")" ]

result="$($SQSH_LS -r --raw "$PWD/escape.squashfs")"
[ "$result" = "$(printf "/dir\33")" ]
