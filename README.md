# libhsqs

squashfs implementation as library.

## Features

*libhsqs* supports all features of the squashfs format. Keep in mind that this
library only supports reading archives. If you need to create archives take a
look at [squashfs-tools-ng](https://github.com/AgentD/squashfs-tools-ng/).

At this point development focuses on avoiding redundant decompressions and
developing efficient cache tactics.

* traverse directories
* read file contents
* open files by path (`hsqs_inode_load_by_path`)
* read metadata from inodes
* read xattr from inodes
* read symlinks from inodes
* read device ids from inodes
* open remote file systems through http (needs *libcurl*)
* ... much more

## Design principles

* Stack oriented structures
* usable in multithreaded applications

## How to...?

### ... open an archive?

```c
struct Hsqs archive = { 0 };
int rv = hsqs_open(&archive, "/path/to/archive.squashfs");
if (rv < 0)
	abort();
// Do something with the archive!
hsqs_cleanup(&archive);
```

### ... get metainformations about a file?

```c
struct HsqsInodeContext inode = { 0 };
int rv = hsqs_inode_load_by_path(&inode, &archive, "/path/to/file");
if (rv < 0)
	abort();
// inode contains metainformations about '/path/to/file'.
// They can be queried with the hsqs_inode_* functions.
// This for example gives you the file size of a file:
int file_size = hsqs_inode_file_size(&inode);
hsqs_inode_cleanup(&inode);
```

## Resource

* https://dr-emann.github.io/squashfs/
* https://dr-emann.github.io/squashfs/squashfs.html
* https://www.kernel.org/doc/Documentation/filesystems/squashfs.txt (useless)
