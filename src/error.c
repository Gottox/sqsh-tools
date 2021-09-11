/**
 * @author      : Enno Boland (mail@eboland.de)
 * @file        : error
 * @created     : Saturday Sep 11, 2021 12:26:48 CEST
 */

#include "error.h"

#include <stdio.h>
#include <string.h>

void
squash_perror(const char *msg, int errorcode) {
	fputs(msg, stderr);
	fputs(squash_error_str(errorcode), stderr);
	fputc('\n', stderr);
}

const char *
squash_error_str(int errorcode) {
	if (errorcode < SQUASH_ERROR_SECTION) {
		return strerror(errorcode);
	}
	switch (-errorcode) {
	case SQUASH_ERROR_SUPERBLOCK_TOO_SMALL:
		return "Superblock Too Small";
	case SQUASH_ERROR_WRONG_MAGIG:
		return "Wrong Magig";
	case SQUASH_ERROR_BLOCKSIZE_MISSMATCH:
		return "Blocksize Missmatch";
	case SQUASH_ERROR_SIZE_MISSMATCH:
		return "Size Missmatch";
	case SQUASH_ERROR_CHECKFLAG_SET:
		return "Checkflag Set";
	case SQUASH_ERROR_METABLOCK_UNSUPPORTED_COMPRESSION:
		return "Metablock Unsupported Compression";
	case SQUASH_ERROR_METABLOCK_INFO_IS_COMPRESSED:
		return "Metablock Info Is Compressed";
	case SQUASH_ERROR_METABLOCK_INIT:
		return "Metablock Init";
	case SQUASH_ERROR_COMPRESSION_INIT:
		return "Compression Init";
	case SQUASH_ERROR_COMPRESSION_DECOMPRESS:
		return "Compression Decompress";
	case SQUASH_ERROR_UNKOWN_INODE_TYPE:
		return "Unkown Inode Type";
	case SQUASH_ERROR_COMPRESSION_STREAM_INIT:
		return "Compression Stream Init";
	case SQUASH_ERROR_COMPRESSION_STREAM_CLEANUP:
		return "Compression Stream Cleanup";
	case SQUASH_ERROR_STREAM_NOT_ENOUGH_BYTES:
		return "Stream Not Enough Bytes";
	case SQUASH_ERROR_GZIP_HEADER_TOO_SMALL:
		return "Gzip Header Too Small";
	case SQUASH_ERROR_GZIP_HEADER_READ:
		return "Gzip Header Read";
	case SQUASH_ERROR_GZIP_READ_AFTER_END:
		return "Gzip Read After End";
	case SQUASH_ERROR_DIRECTORY_WRONG_INODE_TYPE:
		return "Directory Wrong Inode Type";
	case SQUASH_ERROR_MALLOC_FAILED:
		return "Malloc Failed";
	case SQUASH_ERROR_DIRECTORY_INIT:
		return "Directory Init";
	case SQUASH_ERROR_INODE_INIT:
		return "Inode Init";
	case SQUASH_ERROR_INTEGER_OVERFLOW:
		return "Integer Overflow";
	case SQUASH_ERROR_NO_SUCH_FILE:
		return "No Such File";
	case SQUASH_ERROR_TODO:
		return "Todo";
	default:
		return "Unknown Error";
	}
}
