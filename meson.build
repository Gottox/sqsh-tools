project(
	'libhsqs',
	['c'],
	default_options : [
		'c_std=c18',
		'optimization=0',
		'werror=true',
		'warning_level=3'
	],
	version: '0.0.0',
	meson_version: '>=0.57'
)

hsqs_hdr = [
	'src/compression/compression.h',
	'src/context/compression_options_context.h',
	'src/context/content_context.h',
	'src/context/inode_context.h',
	'src/context/metablock_context.h',
	'src/context/metablock_stream_context.h',
	'src/context/superblock_context.h',
	'src/data/compression_options.h',
	'src/data/compression_options_internal.h',
	'src/data/datablock.h',
	'src/data/datablock_internal.h',
	'src/data/directory.h',
	'src/data/directory_internal.h',
	'src/data/fragment.h',
	'src/data/fragment_internal.h',
	'src/data/inode.h',
	'src/data/inode_internal.h',
	'src/data/metablock.h',
	'src/data/metablock_internal.h',
	'src/data/superblock.h',
	'src/data/superblock_internal.h',
	'src/data/xattr.h',
	'src/data/xattr_internal.h',
	'src/error.h',
	'src/hsqs.h',
	'src/iterator/directory_index_iterator.h',
	'src/iterator/directory_iterator.h',
	'src/iterator/xattr_iterator.h',
	'src/mapper/canary_mapper.h',
	'src/mapper/curl_mapper.h',
	'src/mapper/mapper.h',
	'src/mapper/mmap_full_mapper.h',
	'src/mapper/mmap_mapper.h',
	'src/mapper/static_mapper.h',
	'src/primitive/buffer.h',
	'src/table/fragment_table.h',
	'src/table/table.h',
	'src/table/xattr_table.h',
	'src/utils.h',
]

hsqs_src = [
	'src/compression/null.c',
	'src/context/compression_options_context.c',
	'src/context/content_context.c',
	'src/context/inode_context.c',
	'src/context/metablock_context.c',
	'src/context/metablock_stream_context.c',
	'src/context/superblock_context.c',
	'src/data/compression_options.c',
	'src/data/datablock.c',
	'src/data/directory.c',
	'src/data/fragment.c',
	'src/data/inode.c',
	'src/data/metablock.c',
	'src/data/superblock.c',
	'src/data/xattr.c',
	'src/error.c',
	'src/hsqs.c',
	'src/iterator/directory_index_iterator.c',
	'src/iterator/directory_iterator.c',
	'src/iterator/xattr_iterator.c',
	'src/mapper/canary_mapper.c',
	'src/mapper/mapper.c',
	'src/mapper/mmap_full_mapper.c',
	'src/mapper/mmap_mapper.c',
	'src/mapper/static_mapper.c',
	'src/primitive/buffer.c',
	'src/table/fragment_table.c',
	'src/table/table.c',
	'src/table/xattr_table.c',
	'src/utils.c',
]

hsqs_test = [
	'test/integration.c',
]

libhsqs_deps = [ ]

build_args = [
	'-DVERSION="' + meson.project_version() + '"',
	'-D_DEFAULT_SOURCE',
	'-pthread'
]

link_args = [
	'-pthread'
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

if get_option('debug')
	build_args += ['-DDEBUG']
endif

libhsqs = library(
  'hsqs',
  hsqs_src,
  install: not meson.is_subproject(),
  c_args : build_args,
  link_args : link_args,
  include_directories : 'src',
  dependencies : [ libhsqs_deps ],
  sources: hsqs_hdr,
  version : meson.project_version(),
  soversion : meson.project_version().split('.')[0]
)

if get_option('default_library') == 'both'
	libhsqs = libhsqs.get_static_lib()
endif

foreach p : [ 'hsqs-cat', 'hsqs-ls', 'hsqs-stat' ]
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

subdir('doc')
