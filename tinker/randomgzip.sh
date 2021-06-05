#!/usr/bin/env sh

######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : randomgzip
# @created     : Sunday May 30, 2021 09:48:46 CEST
#
# @description : 
######################################################################

max_size=10240

for i in $(seq 1 $max_size); do
	dd if=/dev/urandom bs=$i count=1 | gzip -kn > /tmp/f.gz
	./zlib_test /tmp/f.gz >/dev/null || break
done
