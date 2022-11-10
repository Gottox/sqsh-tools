/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : slice
 * @created     : Wednesday Nov 09, 2022 11:08:10 CET
 */

#include <sqsh_error.h>
#include <sqsh_primitive.h>

SQSH_NO_UNUSED int
sqsh_slice_init(struct SqshSlice *slice, const uint8_t *data, size_t size) {
	slice->data = data;
	slice->size = size;
	return 0;
}

SQSH_NO_UNUSED int
sqsh_slice_init_subslice(
		struct SqshSlice *slice, const struct SqshSlice *source,
		sqsh_index_t offset, size_t size) {
	sqsh_index_t end_offset;

	if (SQSH_ADD_OVERFLOW(offset, size, &end_offset)) {
		return SQSH_ERROR_INTEGER_OVERFLOW;
	}
	if (offset > source->size) {
		return SQSH_ERROR_INDEX_OUT_OF_BOUNDS;
	}
	if (end_offset > source->size) {
		return SQSH_ERROR_INDEX_OUT_OF_BOUNDS;
	}
	return sqsh_slice_init(slice, &source->data[offset], size);
}

SQSH_NO_UNUSED const uint8_t *
sqsh_slice_data(const struct SqshSlice *slice) {
	return slice->data;
}

SQSH_NO_UNUSED size_t
sqsh_slice_size(const struct SqshSlice *slice) {
	return slice->size;
}

SQSH_NO_UNUSED int
sqsh_slice_cleanup(struct SqshSlice *slice) {
	slice->data = NULL;
	slice->size = 0;
	return 0;
}
