#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : large-file-uncompressed.sh
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : creates an archive with a file that fails extracting
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSH_CAT:?SQSH_CAT is not set}"

WORK_DIR="$BUILD_DIR/large-file-uncomressed"

mkdir -p "$WORK_DIR"

cd "$WORK_DIR"

seq 1 8355841 > "$PWD/file"

$MKSQUASHFS "$PWD/file" "$PWD/large-file.squashfs" \
	-comp gzip \
	-all-root \
	-noappend \
	-b 4096 \
	-noI -noD -noF -noX \
	-noappend

$SQSH_CAT "$PWD/large-file.squashfs" "file" | cmp - "$PWD/file"
