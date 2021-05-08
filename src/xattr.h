/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : xattr
 * @created     : Friday May 07, 2021 07:03:51 CEST
 */

#include <stdint.h>

#ifndef XATTR_H

#define XATTR_H

struct SquashXattrKey {
	uint16_t type;
	uint16_t name_size;
	uint8_t name; // [name_size - strlen(prefix)];
};

struct SquashXattrValue {
	uint32_t value_size;
	uint8_t value[0]; // [value_size]
};

struct SquashXattrLookupTable {
	uint64_t xattr_ref;
	uint32_t count;
	uint32_t size;
};

struct SquashXattrIdTable {
	uint64_t xattr_table_start;
	uint32_t xattr_ids;
	uint32_t _unused;
	uint64_t table[0]; // [ceil(xattr_ids / 512.0)]
};

#endif /* end of include guard XATTR_H */
