/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : error
 * @created     : Saturday May 08, 2021 20:03:38 CEST
 */

#ifndef ERROR_H

#define ERROR_H

enum SquashError {
	// Avoid collisions with errno
	SQUASH_SUCCESS = 1 << 8,
	SQUASH_ERROR_CHECKFLAG_SET,
	SQUASH_ERROR_UNSUPPORTED_COMPRESSION,
	SQUASH_ERROR_METABLOCK_INIT,
	SQUASH_ERROR_COMPRESSION_INIT,
	SQUASH_ERROR_UNKOWN_INODE_TYPE,
};

#endif /* end of include guard ERROR_H */
