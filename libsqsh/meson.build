subdir('include')

subdir('src')

if get_option('fuzzer')
    subdir('fuzzer')
endif

libsqsh_dependencies = [
    cextras_dep,
]

libsqsh_c_args = []
if threads_dep.found()
    libsqsh_dependencies += threads_dep
endif

if curl_dep.found()
    libsqsh_dependencies += curl_dep
    libsqsh_c_args += '-DCONFIG_CURL'
endif

if zlib_dep.found()
    libsqsh_dependencies += zlib_dep
    libsqsh_c_args += '-DCONFIG_ZLIB'
endif

if lz4_dep.found()
    libsqsh_dependencies += lz4_dep
    libsqsh_c_args += '-DCONFIG_LZ4'
endif

if lzma_dep.found()
    libsqsh_dependencies += lzma_dep
    libsqsh_c_args += '-DCONFIG_LZMA'
endif

if zstd_dep.found()
    libsqsh_dependencies += zstd_dep
    libsqsh_c_args += '-DCONFIG_ZSTD'
endif

libsqsh = both_libraries(
    'sqsh',
    libsqsh_sources,
    include_directories: [
        libsqsh_private_include,
        libsqsh_common_include,
        libsqsh_include,
    ],
    install: not meson.is_subproject(),
    link_with: [libsqsh_common],
    c_args: libsqsh_c_args,
    dependencies: libsqsh_dependencies,
    version: meson.project_version(),
)

libsqsh_dep = declare_dependency(
    link_with: libsqsh,
    include_directories: libsqsh_include,
)

pkg_mod.generate(
    libsqsh,
    name: 'libsqsh',
    filebase: 'libsqsh',
    description: 'A library to access squashfs archives',
    subdirs: 'sqsh',
    libraries: libsqsh,
    version: meson.project_version(),
)
