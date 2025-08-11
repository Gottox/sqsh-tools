/******************************************************************************
 *                                                                            *
 * Copyright (c) 2023-2024, Enno Boland <g@s01.de>                            *
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
 * @author      Enno Boland (mail@eboland.de)
 * @file        sqshtools_common.h
 */

#ifndef SQSHTOOLS_COMMON_H

#define SQSHTOOLS_COMMON_H

#define _GNU_SOURCE

#include <ctype.h>
#include <getopt.h>
#include <sqsh.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#	define VERSION "0.0.0-unknown"
#endif

struct SqshArchive *
open_archive(const char *image_path, uint64_t offset, int *err);

void print_raw(const char *segment, size_t segment_size);

void print_escaped(const char *segment, size_t segment_size);

void locked_perror(const char *msg);

void locked_sqsh_perror(int error_code, const char *msg);

void locked_perror(const char *msg);

__attribute__((__format__(__printf__, 2, 0))) void
locked_fprintf(FILE *stream, const char *format, ...);

void locked_fputs(const char *s, FILE *stream);

int locked_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif /* SQSHTOOLS_COMMON_H */
