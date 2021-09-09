#!/bin/sh -ex

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : lssquash
# @created     : Saturday Sep 04, 2021 19:06:46 CEST
#
# @description : 
######################################################################

cd $(dirname "$0")

tmp=$(mktemp -d /tmp/lssquash.XXXXXXXXX)

mkdir "$tmp/files"

printf "$tmp/files/%010i\0" $(seq 1 460) | xargs -0 touch

mksquashfs "$tmp/files" $tmp/files.squashfs -noId -noD -noF -noX -nopad -no-xattrs -no-fragments -no-exports -comp zstd > /dev/null 2>&1

../bin/lssquash "$tmp/files.squashfs" > "$tmp/files.list.is"

ls "$tmp/files" | sort > $tmp/files.list.should

diff "$tmp/files.list".{should,is} -upr
