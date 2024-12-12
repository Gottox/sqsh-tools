/******************************************************************************
 *                                                                            *
 * Copyright (c) 2021, Enno Boland <g@s01.de>                                 *
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
 * @author   Enno Boland (mail@eboland.de)
 * @file     common.c
 * @created  Wednesday Jun 07, 2023 15:41:30 CEST
 */

#include "sqsh_error.h"
#include <pthread.h>
#include <sqshtools_common.h>
#include <stdarg.h>

pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;

void
locked_perror(const char *msg) {
	pthread_mutex_lock(&output_lock);
	perror(msg);
	pthread_mutex_unlock(&output_lock);
}

void
locked_fprintf(FILE *stream, const char *format, ...) {
	va_list args;
	va_start(args, format);
	pthread_mutex_lock(&output_lock);
	vfprintf(stream, format, args);
	pthread_mutex_unlock(&output_lock);
	va_end(args);
}

void
locked_fputs(const char *s, FILE *stream) {
	pthread_mutex_lock(&output_lock);
	fputs(s, stream);
	pthread_mutex_unlock(&output_lock);
}

int
locked_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	int ret;
	pthread_mutex_lock(&output_lock);
	ret = fwrite(ptr, size, nmemb, stream);
	pthread_mutex_unlock(&output_lock);
	return ret;
}

void
locked_sqsh_perror(int error_code, const char *msg) {
	pthread_mutex_lock(&output_lock);
	sqsh_perror(error_code, msg);
	pthread_mutex_unlock(&output_lock);
}

struct SqshArchive *
open_archive(const char *image_path, uint64_t offset, int *err) {
	struct SqshConfig config = {
			.source_mapper = NULL,
			.mapper_block_size = 1024 * 256,
			.archive_offset = offset,
	};
	if (sqsh_mapper_impl_curl != NULL) {
		int i;
		for (i = 0; isalnum(image_path[i]); i++) {
		}
		if (strncmp(&image_path[i], "://", 3) == 0) {
			config.source_mapper = sqsh_mapper_impl_curl;
		}
	}

	return sqsh_archive_open(image_path, &config, err);
}

void
print_raw(const char *segment, size_t segment_size) {
	locked_fwrite(segment, 1, segment_size, stdout);
}

void
print_escaped(const char *segment, size_t segment_size) {
	for (size_t i = 0; i < segment_size; i++) {
		switch (segment[i]) {
		case '\n':
			locked_fputs("\\n", stdout);
			break;
		case '\r':
			locked_fputs("\\r", stdout);
			break;
		case '\t':
			locked_fputs("\\t", stdout);
			break;
		case '\\':
			locked_fputs("\\\\", stdout);
			break;
		case '\x1b':
			locked_fputs("\\e", stdout);
			break;
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x0b:
		case 0x0c:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
		case 0x20:
		case 0x7f:
			locked_fprintf(stdout, "\\x%02x", segment[i]);
			break;
		default:
			locked_fputs((char[2]){segment[i], 0}, stdout);
			break;
		}
	}
}
