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
 * @file         sqsh_utils.h
 */

#ifndef SQSH_UTILS_H
#define SQSH_UTILS_H

#include "sqsh_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
 * utils/version.c
 */

/**
 * @brief Get the version of the library that was used to compile the program as
 * a string.
 *
 * @return Version string
 */
#define SQSH_VERSION "1.5.0"

/**
 * @brief Get the major version of the library that was used to compile the
 * program.
 *
 * @return Major version
 */
#define SQSH_VERSION_MAJOR 1

/**
 * @brief Get the minor version of the library that was used to compile the
 * program.
 *
 * @return Minor version
 */
#define SQSH_VERSION_MINOR 5

/**
 * @brief Get the patch version of the library that was used to compile the
 * program.
 *
 * @return Patch version
 */
#define SQSH_VERSION_PATCH 0

/**
 * @brief Get the version of the currently running library as a string.
 *
 * @return Version string
 */
const char *sqsh_version(void);

/**
 * @brief Get the major version of the currently running library.
 *
 * @return Major version
 */
uint16_t sqsh_version_major(void);

/**
 * @brief Get the minor version of the currently running library.
 *
 * @return Minor version
 */
uint16_t sqsh_version_minor(void);

/**
 * @brief Get the patch version of the currently running library.
 *
 * @return Patch version
 */
uint16_t sqsh_version_patch(void);

#ifdef __cplusplus
}
#endif
#endif /* SQSH_UTILS_H */
