project(
    'libsqsh',
    ['c'],
    default_options: [
        'c_std=c11',
        'optimization=0',
        'werror=false',
        'warning_level=3',
    ],
    version: '0.4.0',
    meson_version: '>=0.57',
    license: [
        'BSD-2-Clause',
    ],
)

threads = dependency('threads', required: get_option('threads'))
curl = dependency('libcurl', required: get_option('curl'))
fuse = dependency('fuse3', required: get_option('fuse'))
lz4 = dependency('liblz4', required: get_option('lz4'))
lzma = dependency('liblzma', required: get_option('lzma'))
zlib = dependency('zlib', required: get_option('zlib'))
zstd = dependency('libzstd', required: get_option('zstd'))

subdir('include')

subdir('lib')

subdir('man')

if get_option('default_library') == 'both'
    libsqsh = libsqsh.get_static_lib()
endif

libsqsh_dep = declare_dependency(
    link_with: libsqsh,
    include_directories: libsqsh_include,
)

if get_option('tools')
    subdir('tools')
endif

if get_option('test') != 'false'
    subdir('test')
endif

if get_option('fuzzer')
    subdir('fuzzer')
endif

if get_option('doc') != 'false'
    subdir('doc')
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(
    libsqsh,
    name: 'libsqsh',
    filebase: 'libsqsh',
    description: 'A library to access squashfs archives',
    subdirs: 'sqsh',
    libraries: libsqsh,
    version: meson.project_version(),
)
