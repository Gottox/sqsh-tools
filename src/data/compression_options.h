/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression_options
 * @created     : Monday Sep 06, 2021 11:25:09 CEST
 */

#include <stdint.h>

#ifndef SQUASH__COMPRESSION_OPTIONS_H

#define SQUASH__COMPRESSION_OPTIONS_H

struct SquashCompressionOptionsGzip;

struct SquashCompressionOptionsXz;

struct SquashCompressionOptionsLz4;

struct SquashCompressionOptionsZstd;

struct SquashCompressionOptionsLzo;

union SquashCompressionOptions;

uint32_t squash_compression_options_gzip_compression_level(
		const union SquashCompressionOptions *options);
uint16_t squash_compression_options_gzip_window_size(
		const union SquashCompressionOptions *options);
uint16_t squash_compression_options_gzip_strategies(
		const union SquashCompressionOptions *options);

uint32_t squash_compression_options_xz_dictionary_size(
		const union SquashCompressionOptions *options);
uint32_t squash_compression_options_xz_filters(
		const union SquashCompressionOptions *options);

uint32_t squash_compression_options_lz4_version(
		const union SquashCompressionOptions *options);
uint32_t squash_compression_options_lz4_flags(
		const union SquashCompressionOptions *options);

uint32_t squash_compression_options_zstd_compression_level(
		const union SquashCompressionOptions *options);

uint32_t squash_compression_options_lzo_algorithm(
		const union SquashCompressionOptions *options);
uint32_t squash_compression_options_lzo_compression_level(
		const union SquashCompressionOptions *options);

#endif /* end of include guard SQUASH__COMPRESSION_OPTIONS_H */
