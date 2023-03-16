# organization of the source

### [archive](./archive)

This directory contains the general sqsh archive handling. It contains 
functionality to open/instantiate and close squashfs images.

### [chrome](./chrome)

This directory contains code that eases the usage of this library. It wraps
functionality like opening file contents by path.

### [compression](./compression)

This directory contains the glue code to the different compression algorithms
as well as buffer and cache handling for decompressed data.

### [data](./data)

This directory contains on-disk structures for squashfs. It does not contain 
any logic except for byte order conversion.

### [directory](./directory)

This directory contains code that does directory handling.

### [file](./file)

This directory contains code to handle file contents.

### [inode](./inode)

This directory contains code to handle informations stored in the
[inode table](https://dr-emann.github.io/squashfs/squashfs.html#_inode_table)

### [mapper](./mapper)

This directory takes care of mapping archive contents into memory.
It features multiple backends and can handle local files (through the
[`mmap_mapper`](./mapper/mmap_mapper.c)), in memory archives (through
the [`static_mapper`](./mapper/static_mapper.c)) and remote files (through
the [`curl_mapper`](./mapper/curl_mapper.c)

### [metablock](./metablock)

This directory contains code that takes care of handling
[metadata blocks](https://dr-emann.github.io/squashfs/squashfs.html#_packing_metadata)

### [primitive](./primitive)

This directory contains low level data structures such as dynamic buffers,
hash maps, and lru handling.

### [table](./table)

This directory contains code handling to access generic directly addressable
[lookup tables](https://dr-emann.github.io/squashfs/squashfs.html#_storing_lookup_tables).

### [xattr](./xattr)

This directory contains code for handling and iterating fields in the
[Extended Attribute table](https://dr-emann.github.io/squashfs/squashfs.html#_xattr_table)
