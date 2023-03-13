project(
	'libsqsh',
	['c'],
	default_options : [
		'c_std=c11',
		'optimization=0',
		'werror=true',
		'warning_level=3'
	],
	version: '0.2.0',
	meson_version: '>=0.57'
)

subdir('include')

subdir('lib')

if get_option('default_library') == 'both'
	libsqsh = libsqsh.get_static_lib()
endif

libsqsh_dependency = declare_dependency(
	link_with: libsqsh,
	include_directories: libsqsh_include
)

if get_option('tools')
	subdir('tools')
endif

if get_option('test') != 'false'
	subdir('test')
endif

if get_option('doc') != 'false'
	subdir('doc')
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  libsqsh,
  name : 'libsqsh',
  filebase : 'libsqsh',
  description : 'A library to access squashfs archives',
  subdirs : 'sqsh',
  libraries : libsqsh,
  version : meson.project_version(),
)
