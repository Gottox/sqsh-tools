libsqsh_sources = [
	'archive/archive.c',
	'archive/compression_options_context.c',
	'archive/superblock_context.c',
	'archive/trailing_context.c',
	'chrome/inode.c',
	'chrome/path_resolver.c',
	'compression/buffering_compression.c',
	'compression/compression.c',
	'compression/compression_manager.c',
	'compression/lz4.c',
	'compression/lzma.c',
	'compression/lzo.c',
	'compression/zlib.c',
	'compression/zstd.c',
	'data/compression_options_data.c',
	'data/datablock_data.c',
	'data/directory_data.c',
	'data/fragment_data.c',
	'data/inode_data.c',
	'data/metablock_data.c',
	'data/superblock_data.c',
	'data/xattr_data.c',
	'directory/directory_iterator.c',
	'error.c',
	'file/file_context.c',
	'file/file_reader.c',
	'file/fragment_table.c',
	'inode/directory_index_iterator.c',
	'inode/inode_context.c',
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
	'table/table.c',
	'xattr/xattr_iterator.c',
	'xattr/xattr_table.c',
]

libsqsh_headers_private = [
	'utils.h'
]

libsqsh_dependencies = [
	dependency('threads')
]

c_args = [ ]
if get_option('curl')
	libsqsh_dependencies += dependency('libcurl')
	c_args += '-DCONFIG_CURL'
endif

if get_option('zlib')
	libsqsh_dependencies += dependency('zlib')
	c_args += '-DCONFIG_ZLIB'
endif

if get_option('lz4')
	libsqsh_dependencies += dependency('liblz4')
	c_args += '-DCONFIG_LZ4'
endif

if get_option('lzma')
	libsqsh_dependencies += dependency('liblzma')
	c_args += '-DCONFIG_LZMA'
endif

if get_option('lzo')
	c_args += '-DCONFIG_LZO'
	c_args += '-DCONFIG_LZO_HELPER_PATH="@0@/@1@/sqsh-lzo-helper"'.format(
		get_option('prefix'),
		get_option('libexecdir')
	)
	lzo_helper = executable('sqsh-lzo-helper',
		'compression/lzo-helper.c',
		install: true,
		install_dir: get_option('libexecdir'),
		c_args : c_args,
		dependencies: dependency('lzo2'),
		)
	lzo_helper_path = lzo_helper.full_path()
else
	lzo_helper_path = ''
endif

if get_option('zstd')
	libsqsh_dependencies += dependency('libzstd')
	c_args += '-DCONFIG_ZSTD'
endif

libsqsh = library(
	'sqsh',
	libsqsh_sources + libsqsh_headers_private,
	install: not meson.is_subproject(),
	c_args: c_args,
	dependencies: libsqsh_dependencies,
	version : meson.project_version(),
	soversion : meson.project_version().split('.')[0]
)
