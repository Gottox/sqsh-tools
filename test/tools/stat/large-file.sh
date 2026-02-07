#!/bin/sh -ex

######################################################################
# @author       Enno Boland (mail@eboland.de)
# @file         large-file.sh
#
# @description  Creates an archive with a file >4 GiB to test that
#               file size and block count are reported correctly.
#               This catches 32-bit truncation bugs in
#               sqsh_file_block_count2().
######################################################################

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SQSH_STAT:?SQSH_STAT is not set}"

WORK_DIR="$BUILD_DIR/stat-large-file"

mkdir -p "$WORK_DIR"

cd "$WORK_DIR"

# Create a 5 GiB sparse file and append 100 random bytes so that
# mksquashfs stores the trailing part as a fragment. This triggers the
# has-fragment code path in sqsh_file_block_count2().
truncate -s 5G "$PWD/largefile"
dd if=/dev/urandom bs=100 count=1 >> "$PWD/largefile" 2>/dev/null

$MKSQUASHFS "$PWD/largefile" "$PWD/large-file.squashfs" \
	-comp gzip \
	-all-root \
	-noappend \
	-b 1048576 \
	-always-use-fragments

STAT_OUTPUT=$($SQSH_STAT "$PWD/large-file.squashfs" "largefile")

FILE_SIZE=$(echo "$STAT_OUTPUT" | grep "file size:" | awk '{print $NF}')
BLOCK_COUNT=$(echo "$STAT_OUTPUT" | grep "number of blocks:" | awk '{print $NF}')
HAS_FRAGMENT=$(echo "$STAT_OUTPUT" | grep "has fragment:" | awk '{print $NF}')

EXPECTED_SIZE=5368709220
EXPECTED_BLOCKS=5120

echo "file size: $FILE_SIZE (expected: $EXPECTED_SIZE)"
echo "block count: $BLOCK_COUNT (expected: $EXPECTED_BLOCKS)"
echo "has fragment: $HAS_FRAGMENT (expected: yes)"

test "$FILE_SIZE" = "$EXPECTED_SIZE"
test "$HAS_FRAGMENT" = "yes"
test "$BLOCK_COUNT" = "$EXPECTED_BLOCKS"

rm -r "$WORK_DIR"
