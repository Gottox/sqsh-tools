project(
    'sqsh-tools',
    'c',
    default_options: [
        'c_std=c11',
        'optimization=0',
        'warning_level=3',
    ],
    version: '1.2.0',
    license: 'BSD-2-Clause',
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

extractor_count = 0
extractors = [lz4_dep, lzma_dep, zlib_dep, zlib_dep]
extractors_str = ''
foreach dep : [lz4_dep, lzma_dep, zlib_dep, zlib_dep]
    extractors_str += '\n   - ' + dep.name()
    if dep.found()
        extractor_count += 1
    endif
endforeach

if extractor_count == 0
    warning(
        'No compression library has been enabled. Please make sure you enable at least on of them:' +
        extractors_str,
    )
endif

pkg_mod = import('pkgconfig')

subdir('include')

subdir('common')

subdir('libsqsh')

subdir('libmksqsh')

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
