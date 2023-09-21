/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : common.h
 */

#ifndef TOOLS_COMMON_H

#define TOOLS_COMMON_H

#ifndef _DEFAULT_SOURCE
#	define _DEFAULT_SOURCE
#endif

#include "../include/sqsh.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#	define VERSION "0.0.0-unknown"
#endif

struct SqshArchive *
open_archive(const char *image_path, uint64_t offset, int *err);

#endif /* TOOLS_COMMON_H */
