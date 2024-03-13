subdir('include')

subdir('src')

libsqsh_common_dependencies = [
    cextras_dep,
]

libsqsh_common_c_args = ['-Wconversion', '-Wsign-conversion']

libsqsh_common = static_library(
    'sqshcommon',
    libsqsh_common_sources,
    include_directories: [libsqsh_common_include, libsqsh_include],
    c_args: libsqsh_common_c_args,
    dependencies: libsqsh_common_dependencies,
    install: false,
)

libsqsh_common_dep = declare_dependency(
    link_with: libsqsh_common,
    include_directories: libsqsh_common_include,
)
