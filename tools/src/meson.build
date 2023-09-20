tool_sources = [
    'cat.c',
    'ls.c',
    'stat.c',
    'xattr.c',
    'unpack.c',
]

c_args = ['-DVERSION="@0@"'.format(meson.project_version())]

tools = {}
foreach src : tool_sources
    tool_name = 'sqsh-' + src.split('.')[0]
    tool = executable(
        tool_name,
        [src, 'common.c'],
        c_args: c_args,
        install: not meson.is_subproject(),
        dependencies: libsqsh_dep,
    )
    tools += {tool_name: tool}
endforeach

if fuse3_dep.found()
    tool = executable(
        'sqshfs',
        ['fs3.c', 'common.c', 'fs-common.c'],
        c_args: c_args,
        install: not meson.is_subproject(),
        dependencies: [libsqsh_dep, fuse3_dep],
    )
    tools += {'sqshfs': tool}
endif

if fuse2_dep.found()
    tool = executable(
        'sqshfs2',
        ['fs2.c', 'common.c', 'fs-common.c'],
        c_args: c_args,
        install: not meson.is_subproject() and not fuse3_dep.found(),
        dependencies: [libsqsh_dep, fuse2_dep],
    )
    tools += {'sqshfs2': tool}
endif
