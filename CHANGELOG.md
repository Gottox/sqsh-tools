## v0.6.0

* **[API BREAK]** do not export private symbols to the shared library
* **[API BREAK]** `sqsh_export_table_resolve_inode` gets an inode as `uint64_t`
  instead of `uint32_t`
* **[API BREAK]** `sqsh_inode_file_block_is_compressed` gets index as
  `uint32_t` instead of `int`
* **[API BREAK]** `sqsh_archive_open` gets an extra parameter `int *err` to
  return an error code.
* **[API BREAK]** unexport `sqsh_xattr_table_start` and `sqsh_xattr_table_get`,
  should not be used by library consumers.
* **[API BREAK]** `sqsh_file_content` returns `uint8_t *` instead of `char *`
  should not be used by library consumers.
* Install missing headers
* chrome: Add `sqsh_directory_list()` to list the contents of a directory.
* mapper: add support for files that have an archive embedded in them.
* examples: add example to list the contents of a directory.
* examples: add example to read a file from an archive.


## v0.5.0

* **[API BREAK]** rename SqshInodeCache to SqshInodeMap
* remove lzo compression
* fix C++ support and reenable tests
* `inode_map`: detect inconsistent adds to the inode map
* `directory_iterator`: fix fast indexed lookup
