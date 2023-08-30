libsqsh_sources = files(
    'archive/archive.c',
    'archive/compression_options.c',
    'archive/inode_map.c',
    'archive/superblock.c',
    'archive/trailing_context.c',
    'data/compression_options_data.c',
    'data/directory_data.c',
    'data/fragment_data.c',
    'data/inode_data.c',
    'data/metablock_data.c',
    'data/superblock_data.c',
    'data/xattr_data.c',
    'directory/directory_index_iterator.c',
    'directory/directory_iterator.c',
    'easy/directory.c',
    'easy/file.c',
    'easy/xattr.c',
    'extract/extract_manager.c',
    'extract/extract_view.c',
    'extract/extractor.c',
    'extract/extractor2.c',
    'extract/lz4.c',
    'extract/lzma.c',
    'extract/zlib.c',
    'extract/zstd.c',
    'file/file.c',
    'file/file_iterator.c',
    'file/file_reader.c',
    'file/fragment_view.c',
    'mapper/curl_mapper.c',
    'mapper/map_iterator.c',
    'mapper/map_manager.c',
    'mapper/map_reader.c',
    'mapper/map_slice.c',
    'mapper/mapper.c',
    'mapper/mmap_mapper.c',
    'mapper/static_mapper.c',
    'metablock/metablock_iterator.c',
    'metablock/metablock_reader.c',
    'reader/reader.c',
    'table/export_table.c',
    'table/fragment_table.c',
    'table/id_table.c',
    'table/table.c',
    'table/xattr_table.c',
    'tree/walker.c',
    'utils/error.c',
    'utils/posix.c',
    'utils/thread.c',
    'xattr/xattr_iterator.c',
)

libsqsh_headers_private = ['utils/utils.h']

libsqsh_dependencies = [
    cextras_dep,
]

c_args = []
if threads_dep.found()
    libsqsh_dependencies += threads_dep
endif

if curl_dep.found()
    libsqsh_dependencies += curl_dep
    c_args += '-DCONFIG_CURL'
endif

if zlib_dep.found()
    libsqsh_dependencies += zlib_dep
    c_args += '-DCONFIG_ZLIB'
endif

if lz4_dep.found()
    libsqsh_dependencies += lz4_dep
    c_args += '-DCONFIG_LZ4'
endif

if lzma_dep.found()
    libsqsh_dependencies += lzma_dep
    c_args += '-DCONFIG_LZMA'
endif

if zstd_dep.found()
    libsqsh_dependencies += zstd_dep
    c_args += '-DCONFIG_ZSTD'
endif

libsqsh = both_libraries(
    'sqsh',
    libsqsh_sources + libsqsh_headers_private,
    install: not meson.is_subproject(),
    c_args: c_args,
    dependencies: libsqsh_dependencies,
    version: meson.project_version(),
)
