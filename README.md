# libsqsh [![CI](https://github.com/Gottox/libsqsh/actions/workflows/ci.yaml/badge.svg)](https://github.com/Gottox/libsqsh/actions/workflows/ci.yaml) [![codecov](https://codecov.io/github/Gottox/libsqsh/graph/badge.svg?token=AM5COPDMH0)](https://codecov.io/github/Gottox/libsqsh) [![License](https://img.shields.io/badge/License-BSD_2--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

squashfs is an open and free compressed read-only filesystem. It is used in
embedded devices, live-CDs, or in packaging. The original implementation
resides in the linux kernel, but there are also userspace implementations.

*libsqsh* is a purely 2-Clause BSD Licensed implementation of the squashfs
filesystem in C11. It covers the complete squashfs feature set, while still
being as minimal as possible.

*libsqsh* consists of a C library and a command line tools to interact with
squashfs archives.

Note that *libsqsh* only supports reading squashfs archives. If you want to create
squashfs archives, you can either use
[squashfs-tools](https://github.com/plougher/squashfs-tools/), which provides a
command line interface, or
[squashfs-tools-ng](https://github.com/AgentD/squashfs-tools-ng/), which provides both
a command-line interface and a C library interface.

## Features

* **Complete feature set**: libsqsh supports all features of the squashfs file 
  system.

* **Modern C11 implementation**: Written in modern C11, libsqsh is designed for
  efficiency and compatibility with C++, C, and other languages.

* **High-Level API**: The high-level API provides easy-to-use functions to
  access the contents of a to the contents of a squashfs archive. It is designed
  to be easy to use and fast to develop with.

* **Low-Level API**: The low-level API provides zero-copy interfaces to the 
  squashfs archive. It is designed to be both memory and CPU efficient.

* **Pure 2-Clause-BSD License**: libsqsh is licensed under the 2-Clause BSD
  license. This allows you to use libsqsh in any project, even commercial ones.

## Example

This is a simple example that a) prints the content of a file and b) lists the
content of a directory.

```c
struct SqshArchive *archive =
		sqsh_archive_open("/path/to/archive.squashfs", NULL, NULL);

uint8_t *contents = sqsh_easy_file_content(archive, "/path/to/file", NULL);
assert(contents != NULL);
const size_t size = sqsh_easy_file_size(archive, "/path/to/file", NULL);
fwrite(contents, 1, size, stdout);
free(contents);

char **files = sqsh_easy_directory_list(archive, "/path/to/dir", NULL);
assert(files != NULL);
for (int i = 0; files[i] != NULL; i++) {
	puts(files[i]);
}
free(files);

sqsh_archive_close(archive);
```

Find further examples in the [examples](examples) directory.

## Building

### Dependencies

* **libc**: both [musl](https://www.musl-libc.org/) and
  [glibc](https://www.gnu.org/software/libc/) are supported.
* [**zlib**](https://zlib.net/) *optional*: For gzip compression support.
* [**liblz4**](https://lz4.org/) *optional*: For lz4 compression support.
* [**liblzma**](https://tukaani.org/xz) *optional*: For lzma compression support.
* [**libzstd**](https://facebook.github.io/zstd/) *optional*: For zstd compression
  support.
* [**fuse3**](https://libfuse.github.io/) *optional*: For mounting squashfs
  archives.
* [**fuse2**](https://libfuse.github.io/) *optional*: For mounting squashfs 
  archives on systems that don't support fuse3. e.g. OpenBSD.
* [**libcurl**](https://curl.se/) *optional*: For transparently reading squashfs
  archives from the internet without downloading them first.

Note that to do anything useful with *libsqsh*, you need to have at least one of the
compression libraries enabled.

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

* https://dr-emann.github.io/squashfs/squashfs.html - A detailed description of
  the squashfs file format.
* https://github.com/plougher/squashfs-tools/ - The original squashfs-tools
  implementation.
* https://github.com/AgentD/squashfs-tools-ng/ - A rewrite of the squashfs-tools
  implementation with library support. In contrast to libsqsh, this library is
  able to write squashfs archives.
