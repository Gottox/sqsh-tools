libsqsh_builder_sources = files(
    'archive/superblock_builder.c',
)

libsqsh_builder_headers_private = []

libsqsh_builder_dependencies = [
    cextras_dep,
    libsqsh_dep,
]

c_args = []
if threads_dep.found()
    libsqsh_builder_dependencies += threads_dep
endif

libsqsh_builder = both_libraries(
    'sqsh_builder',
    libsqsh_builder_sources + libsqsh_builder_headers_private,
    #install: not meson.is_subproject(),
    install: false,
    c_args: c_args,
    dependencies: libsqsh_builder_dependencies,
    version: meson.project_version(),
    link_with: libsqsh_data,
)
