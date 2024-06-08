sqsh_extended_test = [
    'selftest.sh',
    'cat/large-file-compressed.sh',
    'cat/large-file-uncompressed.sh',
    'read-chunk/tail.sh',
    'unpack/repack.sh',
    'unpack/pathtraversal/pathtraversal.sh',
    'ls/large-tree.sh',
    'ls/large-tree-lookup.sh',
    'ls/utc.sh',
    'ls/escape.sh',
]
sqsh_extended_fs_test = [
    'fs/large-file.sh',
    'fs/repack.sh',
]

read_chunk = executable(
    'read-chunk',
    'read-chunk.c',
    install: false,
    c_args: test_c_args,
    dependencies: libsqsh_dep,
)

foreach p : sqsh_extended_test
    test(
        p,
        files(p),
        should_fail: sqsh_failing_test.contains(p),
        depends: [
            read_chunk,
            tools['sqsh-ls'],
            tools['sqsh-cat'],
            tools['sqsh-unpack'],
        ],
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
if tools.has_key('sqshfs') and host_machine.system() != 'darwin'
    fs_impl += tools['sqshfs']
endif
if tools.has_key('sqshfs2') and host_machine.system() != 'darwin'
    fs_impl += tools['sqshfs2']
endif

foreach impl : fs_impl
    foreach p : sqsh_extended_fs_test
        test(
            p + ' (' + impl.name() + ')',
            files(p),
            should_fail: sqsh_failing_test.contains(p),
            depends: [
                read_chunk,
                fs_impl,
                tools['sqsh-ls'],
                tools['sqsh-cat'],
                tools['sqsh-unpack'],
            ],
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

