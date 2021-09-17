/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : compression_options
 * @created     : Monday Sep 06, 2021 11:30:35 CEST
 */

#include "compression_options_internal.h"
#include <endian.h>

uint32_t
squash_compression_options_gzip_compression_level(
		const union SquashCompressionOptions *options) {
	return le32toh(options->gzip.compression_level);
}
uint16_t
squash_compression_options_gzip_window_size(
		const union SquashCompressionOptions *options) {
	return le16toh(options->gzip.window_size);
}
uint16_t
squash_compression_options_gzip_strategies(
		const union SquashCompressionOptions *options) {
	return le16toh(options->gzip.strategies);
}

uint32_t
squash_compression_options_xz_dictionary_size(
		const union SquashCompressionOptions *options) {
	return le32toh(options->xz.dictionary_size);
}
uint32_t
squash_compression_options_xz_filters(
		const union SquashCompressionOptions *options) {
	return le32toh(options->xz.filters);
}

uint32_t
squash_compression_options_lz4_version(
		const union SquashCompressionOptions *options) {
	return le32toh(options->lz4.version);
}
uint32_t
squash_compression_options_lz4_flags(
		const union SquashCompressionOptions *options) {
	return le32toh(options->lz4.flags);
}

uint32_t
squash_compression_options_zstd_compression_level(
		const union SquashCompressionOptions *options) {
	return le32toh(options->zstd.compression_level);
}

uint32_t
squash_compression_options_lzo_algorithm(
		const union SquashCompressionOptions *options) {
	return le32toh(options->lzo.algorithm);
}
uint32_t
squash_compression_options_lzo_compression_level(
		const union SquashCompressionOptions *options) {
	return le32toh(options->lzo.compression_level);
}
