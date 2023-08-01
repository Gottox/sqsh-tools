headers = files(
    'sqsh.h',
    'sqsh_archive.h',
    'sqsh_chrome.h',
    'sqsh_common.h',
    'sqsh_data_private.h',
    'sqsh_directory.h',
    'sqsh_directory_private.h',
    'sqsh_error.h',
    'sqsh_extract_private.h',
    'sqsh_file.h',
    'sqsh_file_private.h',
    'sqsh_mapper.h',
    'sqsh_mapper_private.h',
    'sqsh_metablock_private.h',
    'sqsh_primitive_private.h',
    'sqsh_table.h',
    'sqsh_table_private.h',
    'sqsh_thread_private.h',
    'sqsh_xattr.h',
    'sqsh_xattr_private.h',
)

libsqsh_include = include_directories('.')

if not meson.is_subproject()
    install_headers(headers, subdir: 'sqsh')
endif
