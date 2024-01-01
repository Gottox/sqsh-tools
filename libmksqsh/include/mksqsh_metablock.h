/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023, Enno Boland <g@s01.de>                                 *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions are     *
 * met:                                                                       *
 *                                                                            *
 * * Redistributions of source code must retain the above copyright notice,   *
 *   this list of conditions and the following disclaimer.                    *
 * * Redistributions in binary form must reproduce the above copyright        *
 *   notice, this list of conditions and the following disclaimer in the      *
 *   documentation and/or other materials provided with the distribution.     *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS    *
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,  *
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR     *
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR          *
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,      *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,        *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR         *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF     *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING       *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               *
 *                                                                            *
 ******************************************************************************/

/**
 * @author       Enno Boland (mail@eboland.de)
 * @file         sqsh_archive_builder.h
 */

#ifndef MKSQSH_METABLOCK_H
#define MKSQSH_METABLOCK_H

#include <sqsh_data.h>
#include <sqsh_data_set.h>

#include <stdio.h>

/***************************************
 * metablock/metablock_builder.c
 */

struct MksqshMetablock {
	uint8_t buffer[8192];
	uint64_t outer_ref;
	size_t buffer_size;
	FILE *out;
	bool flushed;
};

int mksqsh__metablock_init(struct MksqshMetablock *metablock, FILE *out);

uint64_t mksqsh__metablock_ref(const struct MksqshMetablock *metablock);

int mksqsh__metablock_write(
		struct MksqshMetablock *metablock, const uint8_t *data, size_t size);

bool mksqsh__metablock_was_flushed(const struct MksqshMetablock *metablock);

int mksqsh__metablock_flush(struct MksqshMetablock *metablock);

int mksqsh__metablock_cleanup(struct MksqshMetablock *metablock);

#ifdef __cplusplus
}
#endif
#endif /* MKSQSH_METABLOCK_H */
