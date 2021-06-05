/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : utils
 * @created     : Friday Apr 30, 2021 12:35:31 CEST
 */

#include <stdint.h>

#ifndef UTILS_H

#define UTILS_H

#ifdef __ORDER_LITTLE_ENDIAN__
#define ENSURE_FORMAT_ORDER_16(x) ((void)x)
#define ENSURE_FORMAT_ORDER_32(x) ((void)x)
#define ENSURE_FORMAT_ORDER_64(x) ((void)x)
#define ENSURE_HOST_ORDER_16(x) ((void)x)
#define ENSURE_HOST_ORDER_32(x) ((void)x)
#define ENSURE_HOST_ORDER_64(x) ((void)x)
#else
#define ENSURE_FORMAT_ORDER_16(x) (x) = htole16(x)
#define ENSURE_FORMAT_ORDER_32(x) (x) = htole32(x)
#define ENSURE_FORMAT_ORDER_64(x) (x) = htole64(x)
#define ENSURE_HOST_ORDER_16(x) (x) = le16toh(x)
#define ENSURE_HOST_ORDER_32(x) (x) = le32toh(x)
#define ENSURE_HOST_ORDER_64(x) (x) = le64toh(x)
#endif

uint32_t log2_u32(uint32_t x);

#endif /* end of include guard UTILS_H */
