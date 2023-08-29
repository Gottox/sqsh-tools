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
 * @file         collection.h
 */

#ifndef CEXTRA_ENDIAN_COMPAT_H
#define CEXTRA_ENDIAN_COMPAT_H

#if defined(__APPLE__)
#	include <libkern/OSByteOrder.h>
#	define be16toh(x) OSSwapBigToHostInt16(x)
#	define be32toh(x) OSSwapBigToHostInt32(x)
#	define be64toh(x) OSSwapBigToHostInt64(x)
#	define htobe16(x) OSSwapHostToBigInt16(x)
#	define htobe32(x) OSSwapHostToBigInt32(x)
#	define htobe64(x) OSSwapHostToBigInt64(x)
#	define htole16(x) OSSwapHostToLittleInt16(x)
#	define htole32(x) OSSwapHostToLittleInt32(x)
#	define htole64(x) OSSwapHostToLittleInt64(x)
#	define le16toh(x) OSSwapLittleToHostInt16(x)
#	define le32toh(x) OSSwapLittleToHostInt32(x)
#	define le64toh(x) OSSwapLittleToHostInt64(x)
#elif defined(__FreeBSD__)
#	include <sys/endian.h>
#else
#	include <endian.h>
#endif

#endif /* CEXTRA_ENDIAN_COMPAT_H */
