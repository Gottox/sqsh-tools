tool_sources = [
	'cat.c',
	'ls.c',
	'stat.c',
	'xattr.c',
	'unpack.c',
]

c_args = [ '-DVERSION="@0@"'.format(meson.project_version()) ]

tools = {}
foreach src : tool_sources
	tool_name = 'sqsh-' + src.split('.')[0]
	tool = executable(
		tool_name,
		src,
		c_args: c_args,
		install: not meson.is_subproject(),
		dependencies: libsqsh_dependency
	)
	tools += { tool_name: tool }
endforeach

if get_option('fuse')
	fuse = dependency('fuse3')
	tool = executable(
		'sqsh-mount',
		'mount.c',
		c_args: c_args,
		install: not meson.is_subproject(),
		dependencies: [ libsqsh_dependency, fuse ]
	)
	tools += { 'sqsh-mount': tool }

	tool = executable(
		'sqshfs',
		'fs.c',
		c_args: c_args,
		install: not meson.is_subproject(),
		dependencies: [ libsqsh_dependency, fuse ]
	)
	tools += { 'sqshfs': tool }
endif
