# libhsqs

squashfs implementation as library.

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
