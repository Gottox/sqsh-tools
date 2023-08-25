## [v1.0.0] - 2023-08-23

- **[API BREAK]** do not export private symbols to the shared library
- **[API BREAK]** `sqsh_export_table_resolve_inode` gets an inode as `uint64_t`
  instead of `uint32_t`
- **[API BREAK]** `sqsh_file_file_block_is_compressed` gets index as
  `uint32_t` instead of `int`
- **[API BREAK]** `sqsh_archive_open` gets an extra parameter `int *err` to
  return an error code.
- **[API BREAK]** unexport `sqsh_xattr_table_start` and `sqsh_xattr_table_get`,
  should not be used by library consumers.
- **[API BREAK]** `sqsh_file_content` returns `uint8_t *` instead of `char *`
  should not be used by library consumers.
- Install missing headers
- chrome: Add `sqsh_directory_list()` to list the contents of a directory.
- mapper: add support for files that have an archive embedded in them.
- examples: add example to list the contents of a directory.
- examples: add example to read a file from an archive.
- tree\_walker: Improved symlink handling.
- tree\_walker: Do not duplicate symlink target when resolving paths.
- Fix compilation on FreeBSD (85b4d3d6 by @probonopd)
- tools/unpack: fix extraction of directories without read/list permissions (e118ca58 by @Gottox)
- directory_iterator: fix wrong free, add tests (2e8605a7 by @Gottox)
- reader: reimplementation the reader without `_skip` (e2a58c45 by @Gottox)
- reader: fix handling of when growing a reader (5a5b51ff by @Gottox)
- reader: refactor reader\_fill\_buffer() (2ab7f294 by @Gottox)
- OpenBSD: Fix endless loop when opening certain files in sqshfs2 (2382ef3f by @Gottox)
- reader: remove skip functions from iterators. (59729048 by @Gottox)
- easy/file: fix file exists function (ed2b8b2a by @Gottox)
- refactor: remove primitive, replace by cextras (792bc39c by @Gottox)
- Implement \_skip for data iterators. (ca9ea772 by @Gottox)
- file\_iterator: fix sparse handling (3c33781c by @Gottox)
- curl\_mapper: fix memory leak when initialisation fails (e0862a5a by @Gottox)
- map\_iterator: fix unused initialisation (66421c68 by @Gottox)

## [v0.5.0] - 2032-07-18

- **[API BREAK]** rename SqshInodeCache to SqshInodeMap
- remove lzo compression
- fix C++ support and reenable tests
- `inode_map`: detect inconsistent adds to the inode map
- `directory_iterator`: fix fast indexed lookup
