tools_manpages = [
	'sqsh-cat.1',
	'sqsh-ls.1',
	'sqsh-stat.1',
	'sqsh-unpack.1',
	'sqsh-xattr.1'
]

conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())

foreach template : tools_manpages
	manpage = configure_file(
		input: template + '.in',
		output: template,
		configuration: conf_data
	)
	install_man(manpage)
endforeach

if get_option('fuse')
	manpage = configure_file(
		input: 'sqsh-mount.1.in',
		output: 'sqsh-mount.1',
		configuration: conf_data
	)
	install_man(manpage)
endif
