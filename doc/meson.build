sqsh_doc_src = ['guide.md']

doxygen = find_program('doxygen')
git = find_program('git')
git_version = run_command(
    [
        git,
        '-C', meson.project_source_root(),
        'describe',
        '--always',
        '--tags',
    ],
    capture: true,
    check: true,
).stdout()

conf_data = configuration_data(
    {
        'DOXYGEN_INPUT': meson.project_source_root(),
        'DOXYGEN_OUTPUT': meson.current_build_dir(),
        'DOXYGEN_PROJECT_NUMBER': git_version,
        'DOXYGEN_INTERNAL_DOCS': get_option('doc') == 'internal' ? 'YES' : 'NO',
    },
)

doxyfile = configure_file(
    input: ['Doxyfile.in'],
    output: 'Doxyfile',
    configuration: conf_data,
)

custom_target(
    'doc',
    build_by_default: true,
    command: [doxygen, doxyfile],
    depend_files: sqsh_doc_src + headers + [doxyfile],
    output: ['html'],
)
