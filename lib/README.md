# organization of the source

### [archive](./archive)

This directory contains the general sqsh archive handling. It contains 
functionality to open/instantiate and close squashfs images.

### [data](./data)

This directory contains on-disk structures for squashfs. It does not contain 
any logic except for byte order conversion.

### [directory](./directory)

This directory contains code that does directory handling.

### [easy](./easy)

This directory contains code that eases the usage of this library. It wraps
complex functionality into easy to use functions.

### [extract](./extract)

This directory contains the glue code to the different compression algorithms
as well as buffer and cache handling for decompressed data.

### [file](./file)

This directory contains code to handle informations stored in the
[inode table](https://dr-emann.github.io/squashfs/squashfs.html#_inode_table)
and file contents.

### [mapper](./mapper)

This directory takes care of mapping archive contents into memory.
It features multiple backends and can handle local files (through the
[`mmap_mapper`](./mapper/mmap_mapper.c)), in memory archives (through
the [`static_mapper`](./mapper/static_mapper.c)) and remote files (through
the [`curl_mapper`](./mapper/curl_mapper.c)

### [metablock](./metablock)

This directory contains code that takes care of handling
[metadata blocks](https://dr-emann.github.io/squashfs/squashfs.html#_packing_metadata)

### [reader](./reader)

This directory contains the generic reader code. It is responsible to abstract 
random forward reads on top of memory block iterators like the file iterator,
the map iterator, or the metablock iterator.

### [table](./table)

This directory contains code handling to access generic directly addressable
[lookup tables](https://dr-emann.github.io/squashfs/squashfs.html#_storing_lookup_tables).

### [tree](./table)

This directory contains code abstracting the directory tree and path handling.

### [utils](./utils)

Utility functions that are used throughout the library.

### [xattr](./xattr)

This directory contains code for handling and iterating fields in the
[Extended Attribute table](https://dr-emann.github.io/squashfs/squashfs.html#_xattr_table)
