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
struct SqshArchive archive = sqsh_archive_new("/path/to/archive.squashfs", NULL, &rv);
if (rv < 0)
	abort();
// Do something with the archive!
sqsh_archive_free(archive);
```

### ... get metainformations about a file?

```c
struct SqshInode inode = { 0 };
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

## License

The library is licensed under Simplified BSD License (BSD-2-Clause). The one
exception is `bin/sqsh-lzo-helper.c`, which is licensed under GPL-2.0-only.
This is done to comply with lzo's licence terms.
