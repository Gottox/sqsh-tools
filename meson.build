project(
	'libsqsh',
	['c', 'cpp'],
	default_options : [
		'c_std=c18',
		'optimization=0',
		'werror=true',
		'warning_level=3'
	],
	version: '0.2.0',
	meson_version: '>=0.57'
)

sqsh_hdr = [
	'include/sqsh.h',
	'include/sqsh_common.h',
	'include/sqsh_compression_private.h',
	'include/sqsh_context.h',
	'include/sqsh_context_private.h',
	'include/sqsh_data.h',
	'include/sqsh_data_private.h',
	'include/sqsh_directory.h',
	'include/sqsh_directory_private.h',
	'include/sqsh_error.h',
	'include/sqsh_file.h',
	'include/sqsh_file_private.h',
	'include/sqsh_mapper.h',
	'include/sqsh_mapper_private.h',
	'include/sqsh_metablock_private.h',
	'include/sqsh_primitive_private.h',
	'include/sqsh_table.h',
	'include/sqsh_table_private.h',
	'include/sqsh_xattr.h',
	'include/sqsh_xattr_private.h',
]

sqsh_hdr_priv = [
	'src/utils.h',
]

sqsh_src = [
	'src/compression/compression.c',
	'src/context/compression_options_context.c',
	'src/context/path_resolver_context.c',
	'src/context/superblock_context.c',
	'src/context/trailing_context.c',
	'src/data/compression_options_data.c',
	'src/data/datablock_data.c',
	'src/data/directory_data.c',
	'src/data/fragment_data.c',
	'src/data/inode_data.c',
	'src/data/metablock_data.c',
	'src/data/superblock_data.c',
	'src/data/xattr_data.c',
	'src/directory/directory_iterator.c',
	'src/error.c',
	'src/file/file_context.c',
	'src/file/fragment_table.c',
	'src/inode/directory_index_iterator.c',
	'src/inode/inode_context.c',
	'src/mapper/cursor.c',
	'src/mapper/map_manager.c',
	'src/mapper/mapper.c',
	'src/mapper/mmap_mapper.c',
	'src/mapper/static_mapper.c',
	'src/metablock/metablock_cursor.c',
	'src/metablock/metablock_iterator.c',
	'src/primitive/buffer.c',
	'src/primitive/lru.c',
	'src/primitive/ref_count_array.c',
	'src/sqsh.c',
	'src/table/table.c',
	'src/xattr/xattr_iterator.c',
	'src/xattr/xattr_table.c',
]

sqsh_test = [
	#'test/cpp-test.cpp',
	'test/integration.c',
	'test/mapper/cursor.c',
	'test/metablock/metablock_cursor.c',
	'test/metablock/metablock_iterator.c',
	'test/primitive/buffer.c',
]
sqsh_test_util = [
	'test/util.c',
]

libsqsh_deps = [ ]

build_args = [
	'-pthread'
]

link_args = [
	'-pthread'
]

if get_option('curl')
	libsqsh_deps += dependency('libcurl')
	sqsh_src += 'src/mapper/curl_mapper.c'
	build_args += '-DCONFIG_CURL'
endif

if get_option('zlib')
	libsqsh_deps += dependency('zlib')
	sqsh_src += 'src/compression/zlib.c'
	build_args += '-DCONFIG_ZLIB'
endif

if get_option('lz4')
	libsqsh_deps += dependency('liblz4')
	sqsh_src += 'src/compression/lz4.c'
	build_args += '-DCONFIG_LZ4'
endif

if get_option('lzma')
	libsqsh_deps += dependency('liblzma')
	sqsh_src += 'src/compression/lzma.c'
	sqsh_src += 'src/compression/xz.c'
	build_args += '-DCONFIG_LZMA'
endif

if get_option('lzo2')
	libsqsh_deps += dependency('lzo2')
	sqsh_src += 'src/compression/lzo2.c'
	build_args += '-DCONFIG_LZO'
endif

if get_option('zstd')
	libsqsh_deps += dependency('libzstd')
	sqsh_src += 'src/compression/zstd.c'
	build_args += '-DCONFIG_ZSTD'
endif

if get_option('debug')
	build_args += ['-DDEBUG']
endif

libsqsh = library(
  'sqsh',
  sqsh_src,
  install: not meson.is_subproject(),
  c_args : build_args,
  link_args : link_args,
  include_directories : 'include',
  dependencies : [ libsqsh_deps ],
  sources: sqsh_hdr + sqsh_hdr_priv,
  version : meson.project_version(),
  soversion : meson.project_version().split('.')[0]
)

if get_option('default_library') == 'both'
	libsqsh = libsqsh.get_static_lib()
endif

if get_option('tools')
	tool_build_args = build_args + ['-DVERSION="' + meson.project_version() + '"']
	foreach p : [ 'sqsh-cat', 'sqsh-ls', 'sqsh-stat', 'sqsh-xattr', 'sqsh-unpack' ]
		executable(p, 'bin/'+p+'.c',
			install: not meson.is_subproject(),
			c_args : tool_build_args,
			include_directories : 'include',
			link_with : libsqsh
		)
	endforeach

	if get_option('fuse')
		executable('sqsh-mount',
			'bin/sqsh-mount.c',
			install: not meson.is_subproject(),
			c_args : tool_build_args,
			dependencies: dependency('fuse3'),
			include_directories : 'include',
			link_with : libsqsh
		)
	endif
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
	foreach p : sqsh_test
		t = executable(p.underscorify(),
			[ p, squashfs_h, sqsh_test_util ],
			install : false,
			c_args : build_args,
			include_directories : 'include',
			link_with : libsqsh
		)
		test(p, t)
	endforeach
endif

if get_option('fuzzer')
	executable(
		'simple_fuzzer',
			[ 'fuzzer/simple.c' ],
			install : false,
			include_directories : 'include',
			c_args : build_args + [ '-fsanitize=fuzzer' ],
			link_args : '-fsanitize=fuzzer',
			link_with : libsqsh
		)
endif

libsqsh_dep = declare_dependency(
  link_with : libsqsh,
  include_directories : 'include',
  dependencies : [ libsqsh_deps ],
)

install_headers(
  sqsh_hdr,
  subdir: 'libsqsh'
)

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  libsqsh,
  name : 'libsqsh',
  filebase : 'libsqsh',
  description : 'A library to access squashfs archives',
  subdirs : 'libsqsh',
  libraries : libsqsh,
  version : meson.project_version(),
)

subdir('doc')

clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    run_target(
        'tidy',
        command: [
            clangtidy,
            '-p', meson.project_build_root()
        ] + files(sqsh_src + sqsh_hdr + sqsh_hdr_priv))
endif
