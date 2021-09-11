/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : resolve_path
 * @created     : Friday Sep 10, 2021 10:03:06 CEST
 */

#ifndef RESOLVE_PATH_H

#define RESOLVE_PATH_H

struct SquashInodeContext;
struct SquashSuperblock;

int squash_resolve_path(struct SquashInodeContext *inode,
		const struct SquashSuperblock *superblock, const char *path);

#endif /* end of include guard RESOLVE_PATH_H */
