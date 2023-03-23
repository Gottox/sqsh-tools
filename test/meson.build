sqsh_test = [
	#'cpp-test.cpp',
	'file/file_reader.c',
	'inode/inode.c',
	'integration.c',
	'mapper/map_reader.c',
	'metablock/metablock_reader.c',
	'metablock/metablock_iterator.c',
	'primitive/buffer.c',
	'primitive/lru.c',
	'primitive/rc_map.c',
	'primitive/rc_hash_map.c',
	'compression/compression.c',
	'compression/compression_manager.c',
]
sqsh_test_util = [
	'util.c',
]

mksquashfs = find_program('mksquashfs')
mktemp = find_program('mktemp')
setfattr = find_program('setfattr')
xxd = find_program('xxd')
unshare = find_program('unshare')

squashfs = custom_target(
	'squashfs.image',
	output : 'squashfs.image',
	env: {
		'MKSQUASHFS': mksquashfs.full_path(),
		'SETFATTR': setfattr.full_path(),
	},
	command : ['../utils/create_squashfs.sh', '', '@OUTPUT@', '@PRIVATE_DIR@'],
)

squashfs_h = custom_target(
	'squashfs_image.h',
	input: squashfs,
	output: 'squashfs_image.h',
	capture: true,
	command: [xxd, '-i', '@INPUT@'],
)

foreach p : sqsh_test
	t = executable(p.underscorify(),
		[ p, squashfs_h, sqsh_test_util ],
		install : false,
		dependencies : libsqsh_dependency,
	)
	test(p, t, env: { 'SQSH_LZO_HELPER_PATH': lzo_helper_path })
endforeach

if get_option('test') == 'extended'
	test('selftest',
		files('selftest.sh'),
		timeout: 60,
		env: {
			'BUILD_DIR': meson.current_build_dir(),
			'MKSQUASHFS': mksquashfs.full_path(),
			'SOURCE_ROOT': meson.project_source_root(),
			'SQSH_CAT': tools['sqsh-cat'].full_path(),
			'SQSH_LS': tools['sqsh-ls'].full_path(),
			'SQSH_LZO_HELPER_PATH': lzo_helper_path,
		},
	)

	test('repacktest',
		files('repacktest.sh'),
		timeout: 20,
		should_fail: true,
		env: {
			'BUILD_DIR': meson.current_build_dir(),
			'MKSQUASHFS': mksquashfs.full_path(),
			'SOURCE_ROOT': meson.project_source_root(),
			'SQSH_MOUNT': tools['sqsh-mount'].full_path(),
			'UNSHARE': unshare.full_path(),
		},
	)

	test('create_failing_archive',
		files('create_failing_archive.sh'),
		timeout: 20,
		should_fail: true,
		env: {
			'BUILD_DIR': meson.current_build_dir(),
			'MKSQUASHFS': mksquashfs.full_path(),
			'SOURCE_ROOT': meson.project_source_root(),
			'SQSH_CAT': tools['sqsh-cat'].full_path(),
		},
	)
endif
