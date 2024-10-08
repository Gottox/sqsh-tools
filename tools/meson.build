subdir('man')

subdir('include')

tool_sources = [
    'src/cat.c',
    'src/ls.c',
    'src/stat.c',
    'src/xattr.c',
    'src/unpack.c',
]

tool_dependencies = [
    libsqsh_dep,
    cextras_dep,
]

c_args = ['-DVERSION="@0@"'.format(meson.project_version())]

tools = {}
foreach src : tool_sources
    tool_name = 'sqsh-' + src.split('.')[0].split('/')[-1]
    tool = executable(
        tool_name,
        [src, 'src/common.c'],
        c_args: c_args,
        include_directories: tools_private_include,
        install: not meson.is_subproject(),
        dependencies: tool_dependencies,
    )
    tools += {tool_name: tool}
endforeach

if fuse3_dep.found()
    tool = executable(
        'sqshfs',
        ['src/fs3.c', 'src/common.c', 'src/fs-common.c'],
        c_args: c_args,
        include_directories: tools_private_include,
        install: not meson.is_subproject(),
        dependencies: tool_dependencies + [fuse3_dep],
    )
    tools += {'sqshfs': tool}
endif

if fuse2_dep.found()
    if tools.has_key('sqshfs')
        tool_name = 'sqshfs2'
    else
        tool_name = 'sqshfs'
    endif
    tool = executable(
        tool_name,
        ['src/fs2.c', 'src/common.c', 'src/fs-common.c'],
        c_args: c_args,
        include_directories: tools_private_include,
        install: not meson.is_subproject() and not fuse3_dep.found(),
        dependencies: tool_dependencies + [fuse2_dep],
    )
    tools += {'sqshfs2': tool}
endif
