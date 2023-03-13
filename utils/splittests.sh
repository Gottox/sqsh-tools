#!/bin/sh -e

cd "$(dirname "$0")"
cd "../test"

files='
cpp-test.cpp
integration.c
mapper/map_reader.c
metablock/metablock_reader.c
metablock/metablock_iterator.c
primitive/buffer.c
primitive/lru.c
primitive/rc_map.c
primitive/rc_hash_map.c
compression/compression.c
compression/compression_manager.c
'

format='/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2023, Enno Boland
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         %s.c
 */

%s

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

%s
'

signature_pattern='^[a-z0-9_]*(\(void\|\))'

tmpdir=$(mktemp -d)
for file in $files; do
	echo "Processing $file"
	basename=${file%.*}
	mkdir -p "${tmpdir}/${basename}"
	csplit -f "${tmpdir}/${basename}/" "$file" "/$signature_pattern/" '{*}'
	includes=
	for test in "${tmpdir}/${basename}/"*; do
		if ! signature=$(grep -m1 "$signature_pattern" "$test"); then
			includes=$(grep '^#include \|^$' "$test" | sed 's@^#include "@\0../@')
			continue
		fi
		function=${signature%%(*}
		mkdir -p "${basename}"
		sed -i '1d' "$test"

		printf "$format\n" "$function" "$includes" "$(cat "$test")" \
			> "${basename}/${function}.c"
	done
	rm "$file"
done
