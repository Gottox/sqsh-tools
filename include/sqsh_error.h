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
 * @file         sqsh_error.h
 */

#ifndef SQSH_ERROR_H
#define SQSH_ERROR_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * error.c
 */

/**
 * @brief Error codes for sqsh.
 */
enum SqshError {
	SQSH_SUCCESS = 0,
	/* Avoid collisions with errno */
	SQSH_ERROR_SECTION_START = (1 << 8),
	SQSH_ERROR_SUPERBLOCK_TOO_SMALL,
	SQSH_ERROR_WRONG_MAGIC,
	SQSH_ERROR_BLOCKSIZE_MISMATCH,
	SQSH_ERROR_SIZE_MISMATCH,
	SQSH_ERROR_COMPRESSION_INIT,
	SQSH_ERROR_COMPRESSION_UNSUPPORTED,
	SQSH_ERROR_COMPRESSION_DECOMPRESS,
	SQSH_ERROR_UNKNOWN_FILE_TYPE,
	/**
	 * @deprecated Since 1.1.0. Use SQSH_ERROR_UNKNOWN_FILE_TYPE instead.
	 */
	SQSH_ERROR_UNKOWN_FILE_TYPE __attribute__((deprecated(
			"Since 1.1.0. Use SQSH_ERROR_UNKNOWN_FILE_TYPE instead."))) =
			SQSH_ERROR_UNKNOWN_FILE_TYPE,
	SQSH_ERROR_NOT_A_DIRECTORY,
	SQSH_ERROR_NOT_A_FILE,
	SQSH_ERROR_MALLOC_FAILED,
	SQSH_ERROR_MUTEX_INIT_FAILED,
	SQSH_ERROR_MUTEX_LOCK_FAILED,
	SQSH_ERROR_MUTEX_DESTROY_FAILED,
	SQSH_ERROR_OUT_OF_BOUNDS,
	SQSH_ERROR_INTEGER_OVERFLOW,
	SQSH_ERROR_NO_SUCH_FILE,
	SQSH_ERROR_NO_SUCH_XATTR,
	SQSH_ERROR_NO_FRAGMENT_TABLE,
	SQSH_ERROR_NO_EXTENDED_DIRECTORY,
	SQSH_ERROR_NO_EXPORT_TABLE,
	SQSH_ERROR_NO_XATTR_TABLE,
	SQSH_ERROR_NO_COMPRESSION_OPTIONS,
	SQSH_ERROR_MAPPER_INIT,
	SQSH_ERROR_MAPPER_MAP,
	SQSH_ERROR_CURL_INVALID_RANGE_HEADER,
	SQSL_ERROR_ELEMENT_NOT_FOUND,
	SQSH_ERROR_INVALID_ARGUMENT,
	SQSH_ERROR_WALKER_CANNOT_GO_UP,
	SQSH_ERROR_WALKER_CANNOT_GO_DOWN,
	SQSH_ERROR_CORRUPTED_INODE,
	SQSH_ERROR_CORRUPTED_DIRECTORY_ENTRY,
	SQSH_ERROR_INTERNAL,
	SQSH_ERROR_INODE_MAP_IS_INCONSISTENT,
	SQSH_ERROR_XATTR_SIZE_MISMATCH,
	SQSH_ERROR_UNSUPPORTED_VERSION,
	SQSH_ERROR_TOO_MANY_SYMLINKS_FOLLOWED,
};

/**
 * @brief Print the error message for the given error code.
 *
 * @param error_code The error code.
 * @param msg The message to print before the error message.
 */
void sqsh_perror(int error_code, const char *msg);

/**
 * @brief Get the error message for the given error code.
 *
 * This function is thread safe, but the returned string may be overwritten by
 * the next call to this function on this thread.
 *
 * @param error_code The error code.
 * @return The error message.
 */
SQSH_NO_UNUSED const char *sqsh_error_str(int error_code);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_ERROR_H */
