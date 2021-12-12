project(
	'libhsqs',
	['c'],
	default_options : [
		'c_std=c18',
		'optimization=2',
		'werror=true',
		'warning_level=1'
	],
	version: '0.0.0',
	meson_version: '>=0.57'
)

hsqs_hdr = [
	'src/compression/compression.h',
	'src/compression/buffer.h',
	'src/context/content_context.h',
	'src/context/compression_options_context.h',
	'src/context/metablock_stream_context.h',
	'src/context/directory_context.h',
	'src/context/metablock_context.h',
	'src/context/fragment_context.h',
	'src/context/table_context.h',
	'src/context/xattr_table_context.h',
	'src/context/inode_context.h',
	'src/context/superblock_context.h',
	'src/data/datablock_internal.h',
	'src/data/datablock.h',
	'src/data/fragment_internal.h',
	'src/data/metablock_internal.h',
	'src/data/superblock_internal.h',
	'src/data/inode_internal.h',
	'src/data/xattr_internal.h',
	'src/data/fragment.h',
	'src/data/metablock.h',
	'src/data/superblock.h',
	'src/data/xattr.h',
	'src/data/directory.h',
	'src/data/inode.h',
	'src/data/directory_internal.h',
	'src/data/compression_options.h',
	'src/data/compression_options_internal.h',
	'src/utils/lru_hashmap.h',
	'src/mapper/canary_mapper.h',
	'src/mapper/static_mapper.h',
	'src/mapper/mmap_mapper.h',
	'src/mapper/mmap_full_mapper.h',
	'src/mapper/mapper.h',
	'src/mapper/curl_mapper.h',
	'src/utils.h',
	'src/resolve_path.h',
	'src/hsqs.h',
	'src/error.h',
]

hsqs_src = [
	'src/compression/null.c',
	'src/compression/buffer.c',
	'src/context/directory_context.c',
	'src/context/compression_options_context.c',
	'src/context/metablock_stream_context.c',
	'src/context/metablock_context.c',
	'src/context/table_context.c',
	'src/context/xattr_table_context.c',
	'src/context/inode_context.c',
	'src/context/superblock_context.c',
	'src/context/fragment_context.c',
	'src/context/content_context.c',
	'src/data/compression_options.c',
	'src/data/datablock.c',
	'src/data/fragment.c',
	'src/data/metablock.c',
	'src/data/superblock.c',
	'src/data/directory.c',
	'src/data/inode.c',
	'src/data/xattr.c',
	'src/utils/lru_hashmap.c',
	'src/mapper/canary_mapper.c',
	'src/mapper/mmap_full_mapper.c',
	'src/mapper/mmap_mapper.c',
	'src/mapper/static_mapper.c',
	'src/mapper/mapper.c',
	'src/utils.c',
	'src/resolve_path.c',
	'src/hsqs.c',
	'src/error.c',
]

hsqs_test = [
	'test/integration.c',
	'test/utils/lru_hashmap.c',
]

libhsqs_deps = [ ]

build_args = [
	'-DVERSION="' + meson.project_version() + '"',
	'-D_DEFAULT_SOURCE'
]

if get_option('curl')
	libhsqs_deps += dependency('libcurl')
	hsqs_src += 'src/mapper/curl_mapper.c'
	build_args += '-DCONFIG_CURL'
endif

if get_option('zlib')
	libhsqs_deps += dependency('zlib')
	hsqs_src += 'src/compression/zlib.c'
	build_args += '-DCONFIG_ZLIB'
endif

if get_option('lz4')
	libhsqs_deps += dependency('liblz4')
	hsqs_src += 'src/compression/lz4.c'
	build_args += '-DCONFIG_LZ4'
endif

if get_option('lzma')
	libhsqs_deps += dependency('liblzma')
	hsqs_src += 'src/compression/lzma.c'
	hsqs_src += 'src/compression/xz.c'
	build_args += '-DCONFIG_LZMA'
endif

if get_option('lzo2')
	libhsqs_deps += dependency('lzo2')
	hsqs_src += 'src/compression/lzo2.c'
	build_args += '-DCONFIG_LZO'
endif

if get_option('zstd')
	libhsqs_deps += dependency('libzstd')
	hsqs_src += 'src/compression/zstd.c'
	build_args += '-DCONFIG_ZSTD'
endif

libhsqs = library(
  'hsqs',
  hsqs_src,
  install: not meson.is_subproject(),
  c_args : build_args,
  include_directories : 'src',
  dependencies : libhsqs_deps
)

if get_option('default_library') == 'both'
	libhsqs = libhsqs.get_static_lib()
endif

foreach p : [ 'hsqs-cat', 'hsqs-ls' ]
	executable(p, 'bin/'+p+'.c',
		install: not meson.is_subproject(),
		c_args : build_args,
		link_with : libhsqs
	)
endforeach

if get_option('fuse')
	executable('hsqs-mount',
		'bin/hsqs-mount.c',
		install: not meson.is_subproject(),
		c_args : build_args,
		dependencies: dependency('fuse3'),
		link_with : libhsqs
	)
endif

if get_option('test')
	if get_option('zlib') == false
		error('zlib is needed to run tests')
	endif
	mksquashfs = find_program('mksquashfs')
	mktemp = find_program('mktemp')
	setfattr = find_program('setfattr')
	squashfs = custom_target(
		'squashfs.image',
		output : 'squashfs.image',
		env: {
			'MKSQUASHFS': mksquashfs.full_path(),
			'SETFATTR': setfattr.full_path(),
		},
		command : ['utils/create_squashfs.sh', '', '@OUTPUT@', '@PRIVATE_DIR@'],
	)
	squashfs_h = custom_target(
		'squashfs_image.h',
		input: squashfs,
		output : 'squashfs_image.h',
		command : ['utils/header_from_bin.sh','@INPUT@', '@OUTPUT@', '@PRIVATE_DIR@'],
	)
	foreach p : hsqs_test
		t = executable(p.underscorify(),
			[ p, squashfs_h ],
			install : false,
			c_args : build_args,
			link_with : libhsqs
		)
		test(p, t)
	endforeach
endif
