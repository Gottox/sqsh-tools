# libsqsh
[![CI](https://github.com/Gottox/libsqsh/actions/workflows/ci.yaml/badge.svg)](https://github.com/Gottox/libsqsh/actions/workflows/ci.yaml)

*squashfs* is an open and free compressed read-only file system. It is used in
embedded devices, live CDs, or in packaging. It was originally implemented in
the kernel, but there are several userspace implementations.

*libsqsh* is a pure 2 clause BSD licensed implementation of the squashfs file
system. It covers the complete squashfs feature set and is designed to be fast
and memory efficient.

It is written in modern C11 and can be used with C++ and C code. *libsqsh*
supports a high-level API that focuses on ease of use, and a low-level API that
provides zero-copy interfaces to the squashfs archives.

## Example

```c
struct SqshArchive *archive =
		sqsh_archive_open("/path/to/archive.squashfs", NULL, NULL);

uint8_t *contents = sqsh_easy_file_content(archive, "/path/to/file");
assert(contents != NULL);
const size_t size = sqsh_easy_file_size(archive, "/path/to/file");
fwrite(contents, 1, size, stdout);
free(contents);

char **files =
		sqsh_easy_directory_list(archive, "/path/to/directory", NULL);
assert(files != NULL);
for (int i = 0; files[i] != NULL; i++) {
	printf("%s\n", files[i]);
}
free(files);

sqsh_archive_close(archive);
```

## Roadmap to 1.0

* [x] directory listing
* [x] file content reading
* [x] inode metadata
* [x] path traversal
* [x] xattr support
* [x] symlink resolution for path traversal
* [x] LRU cache for metadata
* [x] LRU cache for file content
* [x] thread safety
* [x] fuse file system
* [x] OpenBSD support
* [x] FreeBSD support
* [ ] refine the high-level API
* [x] refine the low-level API

## Building

### Dependencies

* libc
* libcurl *optional*
* zlib *optional*
* liblz4 *optional*
* liblzma *optional*
* libzstd *optional*
* fuse3 *optional*
* fuse2 *optional*

### Compile & Install

```bash
meson setup build
cd build
meson compile
meson install
```

### LZO2

LZO2 is a fast compression algorithm. Unfortunately the current implementation
is GPL licensed and therefore not included in this library. If you want to use 
LZO2 there's and independent glue library called [libsqsh-lzo](https://github.com/Gottox/libsqsh-lzo).

## Resource

* https://dr-emann.github.io/squashfs/
* https://dr-emann.github.io/squashfs/squashfs.html
* https://www.kernel.org/doc/Documentation/filesystems/squashfs.txt (useless)
