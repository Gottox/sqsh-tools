libsqsh_data_sources = files(
    'compression_options_data.c',
    'directory_data.c',
    'fragment_data.c',
    'inode_data.c',
    'metablock_data.c',
    'superblock_data.c',
    'xattr_data.c',
)

libsqsh_data_dependencies = [
    cextras_dep,
]

libsqsh_data_c_args = ['-Wconversion', '-Wsign-conversion']

libsqsh_data = static_library(
    'sqsh',
    libsqsh_data_sources,
    c_args: libsqsh_data_c_args,
    dependencies: libsqsh_data_dependencies,
    install: false,
)
