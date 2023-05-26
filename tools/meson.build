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
        src,
        c_args: c_args,
        install: not meson.is_subproject(),
        dependencies: libsqsh_dep,
    )
    tools += {tool_name: tool}
endforeach

if fuse.found()
    tool = executable(
        'sqshfs',
        'fs.c',
        c_args: c_args,
        install: not meson.is_subproject(),
        dependencies: [libsqsh_dep, fuse],
    )
    tools += {'sqshfs': tool}
endif
