/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : gzip
 * @created     : Sunday Sep 05, 2021 11:06:57 CEST
 */

#include <stdint.h>
#include <stddef.h>

#ifndef GZIP_H

#define GZIP_H

struct SquashExtractorGzipOptions {
       uint32_t compression_level;
       uint16_t window_size;
       uint16_t strategies;
};

extern const struct SquashExtractorImplementation squash_extractor_gzip;

#endif /* end of include guard GZIP_H */
