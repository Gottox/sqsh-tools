project(
	'libsqsh',
	['c', 'cpp'],
	default_options : [
		'c_std=c11',
		'optimization=0',
		'werror=true',
		'warning_level=3'
	],
	version: '0.2.0',
	meson_version: '>=0.57'
)

sqsh_hdr = [
	'include/sqsh_archive.h',
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
	'src/archive/archive.c',
	'src/archive/compression_options_context.c',
	'src/archive/superblock_context.c',
	'src/compression/buffering_compression.c',
	'src/compression/compression.c',
	'src/compression/compression_manager.c',
	'src/compression/lz4.c',
	'src/compression/lzma.c',
	'src/compression/lzo.c',
	'src/compression/zlib.c',
	'src/compression/zstd.c',
	'src/context/path_resolver_context.c',
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
	'src/mapper/curl_mapper.c',
	'src/mapper/map_reader.c',
	'src/mapper/map_manager.c',
	'src/mapper/mapper.c',
	'src/mapper/map_slice.c',
	'src/mapper/mmap_mapper.c',
	'src/mapper/static_mapper.c',
	'src/metablock/metablock_reader.c',
	'src/metablock/metablock_iterator.c',
	'src/primitive/buffer.c',
	'src/primitive/lru.c',
	'src/primitive/rc_hash_map.c',
	'src/primitive/rc_map.c',
	'src/table/table.c',
	'src/xattr/xattr_iterator.c',
	'src/xattr/xattr_table.c',
]

sqsh_test = [
	#'test/cpp-test.cpp',
	'test/integration.c',
	'test/mapper/map_reader.c',
	'test/metablock/metablock_reader.c',
	'test/metablock/metablock_iterator.c',
	'test/primitive/buffer.c',
	'test/primitive/lru.c',
	'test/primitive/rc_map.c',
	'test/primitive/rc_hash_map.c',
	'test/compression/compression.c',
	'test/compression/compression_manager.c',
]
sqsh_test_util = [
	'test/util.c',
]

libsqsh_deps = [ ]

build_args = [
	'-pthread',
	'-pedantic',
]

link_args = [
	'-pthread'
]

if get_option('curl')
	libsqsh_deps += dependency('libcurl')
	build_args += '-DCONFIG_CURL'
endif

if get_option('zlib')
	libsqsh_deps += dependency('zlib')
	build_args += '-DCONFIG_ZLIB'
endif

if get_option('lz4')
	libsqsh_deps += dependency('liblz4')
	build_args += '-DCONFIG_LZ4'
endif

if get_option('lzma')
	libsqsh_deps += dependency('liblzma')
	build_args += '-DCONFIG_LZMA'
endif

if get_option('lzo')
	build_args += '-DCONFIG_LZO'
	build_args += '-DCONFIG_LZO_HELPER_PATH="@0@/@1@/sqsh-lzo-helper"'.format(get_option('prefix'), get_option('libexecdir'))
	lzo_helper = executable('sqsh-lzo-helper',
		'bin/sqsh-lzo-helper.c',
		install: true,
		install_dir: get_option('libexecdir'),
		c_args : build_args,
		dependencies: dependency('lzo2'),
	)
	lzo_helper_path = lzo_helper.full_path()
else
	lzo_helper_path = ''
endif

if get_option('zstd')
	libsqsh_deps += dependency('libzstd')
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

tools = {}
if get_option('tools')
	tool_build_args = build_args + ['-DVERSION="' + meson.project_version() + '"']
	foreach p : [ 'sqsh-cat', 'sqsh-ls', 'sqsh-stat', 'sqsh-xattr', 'sqsh-unpack' ]
		tool = executable(p, 'bin/'+p+'.c',
			install: not meson.is_subproject(),
			c_args : tool_build_args,
			include_directories : 'include',
			link_with : libsqsh
		)
		tools += { p : tool }
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

if get_option('test') != 'false'
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
		test(p, t, env: { 'SQSH_LZO_HELPER_PATH': lzo_helper_path })
	endforeach
	if get_option('test') == 'extended'
	test('selftest', files('test/selftest.sh'), env: {
			'SQSH_LZO_HELPER_PATH': lzo_helper_path,
			'SQSH_LS': tools['sqsh-ls'].full_path(),
			'SQSH_CAT': tools['sqsh-cat'].full_path(),
			'SOURCE_ROOT': meson.project_source_root(),
		},
		timeout: 60,
	)
	endif
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
