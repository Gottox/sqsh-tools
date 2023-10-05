/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : common.h
 */

#ifndef TOOLS_COMMON_H

#define TOOLS_COMMON_H

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

#endif /* TOOLS_COMMON_H */
