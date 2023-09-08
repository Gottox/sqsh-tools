subdir('data')

subdir('read')

libsqsh_dep = declare_dependency(
    link_with: libsqsh,
    include_directories: libsqsh_include,
)

if get_option('test') != 'false'
    subdir('build')

    libsqsh_builder_dep = declare_dependency(
        link_with: libsqsh_builder,
        include_directories: libsqsh_include,
        dependencies: [libsqsh_dep],
    )
endif
