subdir('src')

subdir('include')

libmksqsh_dependencies = [
    cextras_dep,
    libsqsh_dep,
    libsqsh_common_dep,
]

c_args = []
if threads_dep.found()
    libmksqsh_dependencies += threads_dep
endif

libmksqsh = both_libraries(
    'mksqsh',
    libmksqsh_sources,
    include_directories: [
        libsqsh_private_include,
        libmksqsh_include,
    ],

    install: false,
    c_args: c_args,
    dependencies: libmksqsh_dependencies,
    version: meson.project_version(),
)
