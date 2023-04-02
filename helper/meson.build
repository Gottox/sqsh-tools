if lzo.found()
	lzo_helper = executable('sqsh-lzo-helper',
		'compression/lzo-helper.c',
		install: true,
		install_dir: get_option('libexecdir'),
		dependencies: lzo,
	)
	lzo_helper_path = lzo_helper.full_path()
else
	lzo_helper_path = ''
endif
