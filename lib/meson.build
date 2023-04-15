libsqsh_sources = [
    'archive/archive.c',
    'archive/compression_options.c',
    'archive/superblock.c',
    'archive/trailing_context.c',
    'chrome/file.c',
    'chrome/inode.c',
    'data/compression_options_data.c',
    'data/directory_data.c',
    'data/fragment_data.c',
    'data/inode_data.c',
    'data/metablock_data.c',
    'data/superblock_data.c',
    'data/xattr_data.c',
    'directory/directory_iterator.c',
    'error.c',
    'extract/extract_buffer.c',
    'extract/extract_manager.c',
    'extract/extract_view.c',
    'extract/extractor.c',
    'extract/lz4.c',
    'extract/lzma.c',
    'extract/lzo.c',
    'extract/zlib.c',
    'extract/zstd.c',
    'file/file_iterator.c',
    'file/file_reader.c',
    'file/fragment_view.c',
    'inode/directory_index_iterator.c',
    'inode/inode.c',
    'mapper/curl_mapper.c',
    'mapper/map_manager.c',
    'mapper/map_reader.c',
    'mapper/map_slice.c',
    'mapper/mapper.c',
    'mapper/mmap_mapper.c',
    'mapper/static_mapper.c',
    'metablock/metablock_iterator.c',
    'metablock/metablock_reader.c',
    'primitive/buffer.c',
    'primitive/lru.c',
    'primitive/rc_hash_map.c',
    'primitive/rc_map.c',
    'table/export_table.c',
    'table/fragment_table.c',
    'table/id_table.c',
    'table/table.c',
    'table/xattr_table.c',
    'tree/walker.c',
    'xattr/xattr_iterator.c',
]

libsqsh_headers_private = ['utils.h']

libsqsh_dependencies = []

c_args = []
if threads.found()
    libsqsh_dependencies += threads
endif

if curl.found()
    libsqsh_dependencies += curl
    c_args += '-DCONFIG_CURL'
endif

if zlib.found()
    libsqsh_dependencies += zlib
    c_args += '-DCONFIG_ZLIB'
endif

if lz4.found()
    libsqsh_dependencies += lz4
    c_args += '-DCONFIG_LZ4'
endif

if lzma.found()
    libsqsh_dependencies += lzma
    c_args += '-DCONFIG_LZMA'
endif

if lzo.found()
    c_args += '-DCONFIG_LZO'
    c_args += '-DCONFIG_LZO_HELPER_PATH="@0@/@1@/sqsh-lzo-helper"'.format(
        get_option('prefix'),
        get_option('libexecdir'),
    )
endif

if zstd.found()
    libsqsh_dependencies += zstd
    c_args += '-DCONFIG_ZSTD'
endif

libsqsh = library(
    'sqsh',
    libsqsh_sources + libsqsh_headers_private,
    install: not meson.is_subproject(),
    c_args: c_args,
    dependencies: libsqsh_dependencies,
    version: meson.project_version(),
    soversion: meson.project_version().split('.')[0],
)
