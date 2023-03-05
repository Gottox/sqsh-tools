bin_sources = [
	'sqsh-cat.c',
	'sqsh-ls.c',
	'sqsh-stat.c',
	'sqsh-xattr.c',
	'sqsh-unpack.c',
]

c_args = [ '-DVERSION="@0@"'.format(meson.project_version()) ]

bins = []
foreach src : bin_sources
	bin_name = src.split('.')[0]
	bin = executable(
		bin_name,
		src,
		c_args: c_args,
		install: not meson.is_subproject(),
		dependencies: libsqsh_dependency
	)
	bins += { bin_name: bin }
endforeach

if get_option('fuse')
	fuse = dependency('fuse3')
	bin = executable(
		'sqsh-mount',
		'sqsh-mount.c',
		c_args: c_args,
		install: not meson.is_subproject(),
		dependencies: [ libsqsh_dependency, fuse ]
	)
endif
