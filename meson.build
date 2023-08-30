project(
    'libsqsh',
    'c',
    default_options: [
        'c_std=c11',
        'optimization=0',
        'warning_level=3',
    ],
    version: '1.0.0',
    license: [
        'BSD-2-Clause',
    ],
)

threads_dep = dependency('threads', required: get_option('threads'))
curl_dep = dependency(
    'libcurl',
    version: '>=7.83.0',
    required: get_option('curl'),
)
fuse3_dep = dependency(
    'fuse3',
    required: get_option('fuse'),
)
fuse2_dep = dependency(
    'fuse',
    version: '>=2.8.0',
    required: get_option('fuse-old'),
)
lz4_dep = dependency('liblz4', required: get_option('lz4'))
lzma_dep = dependency('liblzma', required: get_option('lzma'))
zlib_dep = dependency('zlib', required: get_option('zlib'))
zstd_dep = dependency('libzstd', required: get_option('zstd'))
cextras_dep = subproject('cextras').get_variable('cextras_dep')

subdir('include')

subdir('lib')

subdir('man')

libsqsh_dep = declare_dependency(
    link_with: libsqsh,
    include_directories: libsqsh_include,
)

if get_option('examples')
    subdir('examples')
endif

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
