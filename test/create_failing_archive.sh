#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : repacktest
# @created     : Friday Mar 17, 2023 15:11:09 CET
#
# @description : creates an archive with a file that fails extracting
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSH_CAT:?SQSH_CAT is not set}"

cd "$SOURCE_ROOT"

tmpdir="$BUILD_DIR/create_failing_archive"
mkdir -p "$tmpdir"

seq 1 8355841 | grep ".$" -o | tr -d '\n' > "$tmpdir/file"

$MKSQUASHFS "$tmpdir/file" "$tmpdir/image" \
	-comp gzip \
	-all-root \
	-noappend \
	-b 4096 \
	-no-compression \
	-noappend

exec $SQSH_CAT "$tmpdir/image" "$tmpdir/file"
