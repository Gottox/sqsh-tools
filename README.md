# libsqsh
[![CI](https://github.com/Gottox/libsqsh/actions/workflows/ci.yaml/badge.svg)](https://github.com/Gottox/libsqsh/actions/workflows/ci.yaml)

squashfs is an open and free compressed read-only filesystem. It is used in
embedded devices, live-CDs, or in packaging. It was original introduced into
the kernel, but there are multiple user space implementations.

libsqsh is a purely 2-Clause BSD Licensed implementation of the squashfs
filesystem. It covers the complete squashfs feature set, while still being
as minimal as possible.

## Features

*libsqsh* supports all features of the squashfs format. Keep in mind that this
library only supports reading archives. If you need to create archives take a
look at [squashfs-tools-ng](https://github.com/AgentD/squashfs-tools-ng/).

libsqsh is portable! It is tested and designed for posix compatible environments
but should be easy to port to other systems too. With
[sqsh.js](https://github.com/Gottox/sqsh.js) there's a wasm implementation of
libsqsh with a Javascript API designed to run in browsers.

At this point development focuses on avoiding redundant decompressions and
developing efficient cache tactics.

* traverse directories
* read file contents
* open files by path
* fast filename lookup
* read metadata from inodes
* read xattr from inodes
* read symlinks from inodes
* read device ids from inodes
* open remote file systems through http (needs *libcurl*)
* ... much more

### LZO2

LZO2 is a fast compression algorithm. Unfortunately the current implementation
is GPL licensed and therefore not included in this library. If you want to use 
LZO2 there's and independent glue library called [libsqsh-lzo](https://github.com/Gottox/libsqsh-lzo).

## building

### dependencies

* libc
* libcurl *optional*
* zlib *optional*
* liblz4 *optional*
* liblzma *optional*
* libzstd *optional*
* fuse3 *optional*

### compile & install

```bash
meson setup build
cd build
meson compile
meson install
```

## Example

```c
int rv;
struct SqshArchive archive = sqsh_archive_new("/path/to/archive.squashfs", NULL, &rv);
assert(rv == 0);
struct SqshInode *file = sqsh_open(&archive, "/path/to/file", &rv);
assert(rv == 0);
struct SqshFileIterator *iterator = sqsh_file_iterator_new(file, &rv)
assert(rv == 0);
while(sqsh_file_iterator_next(iterator, 1) > 0) {
	const uint8_t *data = sqsh_file_iterator_data(iterator);
	size_t size = sqsh_file_iterator_size(iterator);
	printf("Chunk Size: %lu\n", size);
	puts("Data:");
	fwrite(data, 1, size, stdout);
}
sqsh_file_iterator_free(iterator);
sqsh_close(file);
sqsh_archive_free(archive);
```

## Resource

* https://dr-emann.github.io/squashfs/
* https://dr-emann.github.io/squashfs/squashfs.html
* https://www.kernel.org/doc/Documentation/filesystems/squashfs.txt (useless)
