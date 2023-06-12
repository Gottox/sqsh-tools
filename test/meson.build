add_languages('cpp', native: false)

sqsh_test = [
    'cpp-test.cpp',
    'ansi.c',
    'directory/directory_iterator.c',
    'extract/extract.c',
    'extract/extract_manager.c',
    'file/file_iterator.c',
    'file/file_reader.c',
    'fuzzer_repro.c',
    'inode/inode.c',
    'integration.c',
    'mapper/map_iterator.c',
    'mapper/map_reader.c',
    'metablock/metablock_iterator.c',
    'metablock/metablock_reader.c',
    'primitive/buffer.c',
    'primitive/lru.c',
    'primitive/rc_hash_map.c',
    'primitive/rc_map.c',
    'primitive/reader.c',
    'tree/walker.c',
    'xattr/xattr_iterator.c',
]
sqsh_test_extra_args = {
    'ansi.c': '-ansi',
}
sqsh_extended_test = [
    'selftest.sh',
    'tools/cat/large-file-compressed.sh',
    'tools/cat/large-file-uncompressed.sh',
    'tools/read-chunk/tail.sh',
    'tools/unpack/repack.sh',
]
sqsh_extended_fs_test = [
    'tools/fs/large-file.sh',
    'tools/fs/repack.sh',
]
sqsh_extra_source = {
    'fuzzer_repro.c': '../fuzzer/simple.c',
}
sqsh_failing_test = []
sqsh_test_util = [
    'util.c',
]
test_c_args = []

cc = meson.get_compiler('c')
if cc.get_id() == 'gcc'
    test_c_args += '-Wno-pedantic'
elif cc.get_id() == 'clang'
    test_c_args += '-Wno-gnu-designator'
endif

mksquashfs = find_program('mksquashfs')
mksquashfs_version = run_command(mksquashfs, ['-version'], capture: true, check: true).stdout().split(' ')[2]
if mksquashfs_version.version_compare('<4.6')
    error('mksquashfs version 4.6 or newer is required. Installed: ' + mksquashfs_version)
endif
squashfs = custom_target(
    'squashfs.image',
    output: 'squashfs.image',
    env: {
        'MKSQUASHFS': mksquashfs.full_path(),
    },
    command: ['./integration_create.sh', '', '@OUTPUT@', '@PRIVATE_DIR@'],
)

squashfs_h = custom_target(
    'squashfs_image.c',
    input: squashfs,
    output: 'squashfs_image.c',
    command: ['../utils/f2h.sh', 'squashfs_image', '@INPUT@', '@OUTPUT@'],
)
sqsh_extra_source += {
    'integration.c': squashfs_h,
}

read_chunk = executable(
    'read-chunk',
    'read-chunk.c',
    install: false,
    c_args: test_c_args,
    dependencies: libsqsh_dep,
)

foreach p : sqsh_test
    sources = [p, sqsh_test_util]
    if sqsh_extra_source.has_key(p)
        sources += sqsh_extra_source[p]
    endif
    this_c_args = []
    this_c_args += test_c_args
    if sqsh_test_extra_args.has_key(p)
        this_c_args += sqsh_test_extra_args[p]
    endif
    t = executable(
        p.underscorify(),
        sources,
        install: false,
        c_args: this_c_args,
        dependencies: [libsqsh_dep, threads_dep],
    )
    test(p, t)
endforeach

if get_option('test') == 'extended'
    foreach p : sqsh_extended_test
        test(
            p,
            files(p),
            should_fail: sqsh_failing_test.contains(p),
            env: {
                'BUILD_DIR': meson.current_build_dir(),
                'SOURCE_ROOT': meson.project_source_root(),
                'MKSQUASHFS': mksquashfs.full_path(),
                'READ_CHUNK': read_chunk.full_path(),
                'SQSH_CAT': tools['sqsh-cat'].full_path(),
                'SQSH_LS': tools['sqsh-ls'].full_path(),
                'SQSH_UNPACK': tools['sqsh-unpack'].full_path(),
            },
        )
    endforeach

    fs_impl = []
    if fuse3_dep.found()
        fs_impl += tools['sqshfs']
    endif
    if fuse2_dep.found()
        fs_impl += tools['sqshfs2']
    endif

    foreach impl : fs_impl
        foreach p : sqsh_extended_fs_test
            test(
                p + ' (' + impl.name() + ')',
                files(p),
                should_fail: sqsh_failing_test.contains(p),
                env: {
                    'SQSHFS': impl.full_path(),
                    'BUILD_DIR': meson.current_build_dir(),
                    'SOURCE_ROOT': meson.project_source_root(),
                    'MKSQUASHFS': mksquashfs.full_path(),
                    'READ_CHUNK': read_chunk.full_path(),
                    'SQSH_CAT': tools['sqsh-cat'].full_path(),
                    'SQSH_LS': tools['sqsh-ls'].full_path(),
                    'SQSH_UNPACK': tools['sqsh-unpack'].full_path(),
                },
            )
        endforeach
    endforeach
endif
