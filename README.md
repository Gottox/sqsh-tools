# libsqsh

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

At this point development focuses on avoiding redundant decompressions and
developing efficient cache tactics.

* traverse directories
* read file contents
* open files by path (`sqsh_inode_load_by_path`)
* fast filename lookup
* read metadata from inodes
* read xattr from inodes
* read symlinks from inodes
* read device ids from inodes
* open remote file systems through http (needs *libcurl*)
* ... much more

## building

### dependencies

* libc
* libcurl *optional*
* zlib *optional*
* liblz4 *optional*
* liblzma *optional*
* lzo2 *optional*
* libzstd *optional*
* fuse3 *optional*

### compile & install

```bash
meson setup build
cd build
meson compile
meson install
```

## How to...?

### ... open an archive?

```c
int rv;
struct Sqsh archive = sqsh_new("/path/to/archive.squashfs", NULL, &rv);
if (rv < 0)
	abort();
// Do something with the archive!
sqsh_free(archive);
```

### ... get metainformations about a file?

```c
struct SqshInodeContext inode = { 0 };
int rv = sqsh_inode_load_by_path(&inode, &archive, "/path/to/file");
if (rv < 0)
	abort();
// inode contains metainformations about '/path/to/file'.
// They can be queried with the sqsh_inode_* functions.
// This for example gives you the file size of a file:
int file_size = sqsh_inode_file_size(&inode);
sqsh_inode_cleanup(&inode);
```

## Resource

* https://dr-emann.github.io/squashfs/
* https://dr-emann.github.io/squashfs/squashfs.html
* https://www.kernel.org/doc/Documentation/filesystems/squashfs.txt (useless)

## Design principles

* Stack located structures
* usable in multithreaded applications

## organisation of source

* *src/primitive*: basic building blocks like buffer management or reference
  counting
* *src/mapper*: implementations that load segments of squashfs file into
  memory. Most notably the mmap management happens here.
* *src/data*: The actual model of a squashfs archive.
* *src/context*: Implements the high level logic on how to retrieve informations
  from the squashfs archive
* *src/iterator*: Implementations of squashfs structures that are read linear.
* *src/table*: Implementation of squashfs structures that can be accessed by
  an index.
* *src/compression*: Gluecode for the different compression algorithms.

The user is supposed to interact with structures of *src/context* and *src/iterator*
modules. 
