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
 * @author       Enno Boland (mail@eboland.de)
 * @file         sqsh_mapper.h
 */

#ifndef SQSH_MAPPER_H
#define SQSH_MAPPER_H

#include <sqsh_common.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * mapper/mapper.c
 */

struct SqshMapper;

/**
 * @brief The implementation of a memory mapper.
 */
struct SqshMemoryMapperImpl {
	/**
	 * @brief A hint to libsqsh to use this block size if the user did not
	 * specify one.
	 */
	size_t block_size_hint;
	/**
	 * @brief The initialization function for the mapper. Use
	 * sqsh_mapper_set_user_data() to set custom user data.
	 */
	int (*init)(struct SqshMapper *mapper, const void *input, size_t *size);
	/**
	 * @brief The function that maps a block of data into memory.
	 */
	int (*map)(
			const struct SqshMapper *mapper, sqsh_index_t offset, size_t size,
			uint8_t **data);
	/**
	 * @brief The function that unmaps a block of data from memory.
	 */
	int (*unmap)(const struct SqshMapper *mapper, uint8_t *data, size_t size);
	/**
	 * @brief The cleanup function for the mapper.
	 */
	int (*cleanup)(struct SqshMapper *mapper);
};

/**
 * @memberof SqshMapper
 * @brief Sets the user data for a mapper.
 *
 * @param[in] mapper The mapper to set the user data for.
 * @param[in] user_data The user data to set.
 */
void sqsh_mapper_set_user_data(struct SqshMapper *mapper, void *user_data);

/**
 * @memberof SqshMapper
 * @brief Retrieves the user data from a mapper.
 *
 * @param[in] mapper The mapper to retrieve the user data from.
 *
 * @return The user data from the mapper.
 */
void *sqsh_mapper_user_data(const struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Retrieves the size of the input data in a mapper.
 *
 * @param[in] mapper The mapper to retrieve the size from.
 *
 * @return The size of the input data in the mapper.
 */
size_t sqsh_mapper_size(const struct SqshMapper *mapper);

/**
 * @internal
 * @memberof SqshMapper
 * @brief Retrieves the block size for a mapper.
 *
 * @param[in] mapper The mapper to retrieve the size from.
 *
 * @return The size of the input data in the mapper.
 */
size_t sqsh_mapper_block_size(const struct SqshMapper *mapper);

/***************************************
 * mapper/curl_mapper.c
 */

/**
 * @brief a mapper that uses curl to download the file.
 */
extern const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_curl;

/***************************************
 * mapper/mmap_mapper.c
 */

/**
 * @brief a mapper that uses mmap to map the file into memory.
 */
extern const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_mmap;

/***************************************
 * mapper/static_mapper.c
 */

/**
 * @brief a mapper that uses a static buffer.
 */
extern const struct SqshMemoryMapperImpl *const sqsh_mapper_impl_static;

#ifdef __cplusplus
}
#endif
#endif /* SQSH_MAPPER_H */
