# organization of the source

## [data](./data)

This directory contains on-disk structures for squashfs. It does not contain 
any logic except for byte order conversion.

## [read](./read)

This directory contains the code to read squashfs archives. It is split into
multiple modules.

### [read/archive](./read/archive)

This directory contains the general sqsh archive handling. It contains 
functionality to open/instantiate and close squashfs images.

### [read/directory](./read/directory)

This directory contains code that does directory handling.

### [read/easy](./read/easy)

This directory contains code that eases the usage of this library. It wraps
complex functionality into easy to use functions.

### [read/extract](./read/extract)

This directory contains the glue code to the different compression algorithms
as well as buffer and cache handling for decompressed data.

### [read/file](./read/file)

This directory contains code to handle informations stored in the
[read/inode table](https://dr-emann.github.io/squashfs/squashfs.html#_inode_table)
and file contents.

### [read/mapper](./read/mapper)

This directory takes care of mapping archive contents into memory.
It features multiple backends and can handle local files (through the
[`mmap_mapper`](./read/mapper/mmap_mapper.c)), in memory archives (through
the [`static_mapper`](./read/mapper/static_mapper.c)) and remote files (through
the [`curl_mapper`](./read/mapper/curl_mapper.c)

### [read/metablock](./read/metablock)

This directory contains code that takes care of handling
[metadata blocks](https://dr-emann.github.io/squashfs/squashfs.html#_packing_metadata)

### [read/reader](./read/reader)

This directory contains the generic reader code. It is responsible to abstract 
random forward reads on top of memory block iterators like the file iterator,
the map iterator, or the metablock iterator.

### [read/table](./read/table)

This directory contains code handling to access generic directly addressable
[lookup tables](https://dr-emann.github.io/squashfs/squashfs.html#_storing_lookup_tables).

### [read/tree](./read/table)

This directory contains code abstracting the directory tree and path handling.

### [read/utils](./read/utils)

Utility functions that are used throughout the library.

### [read/xattr](./read/xattr)

This directory contains code for handling and iterating fields in the
[Extended Attribute table](https://dr-emann.github.io/squashfs/squashfs.html#_xattr_table)
