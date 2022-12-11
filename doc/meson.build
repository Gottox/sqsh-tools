if get_option('doc') != 'false'
  doxygen = find_program('doxygen')
  git = find_program('git')
  git_version = run_command(
    [git, '-C', meson.project_source_root(), 'describe', '--always', '--tags'],
    capture : true,
    check : true
  ).stdout()
  conf_data = configuration_data()
  conf_data.set('DOXYGEN_INPUT', meson.project_source_root())
  conf_data.set('DOXYGEN_OUTPUT', meson.current_build_dir())
  conf_data.set('DOXYGEN_PROJECT_NUMBER', git_version)
  if get_option('doc') == 'internal'
    conf_data.set('DOXYGEN_INTERNAL_DOCS', 'YES')
  else
    conf_data.set('DOXYGEN_INTERNAL_DOCS', 'NO')
  endif

  doxyfile = configure_file(
    input: [ 'Doxyfile.in' ],
    output: 'Doxyfile',
    configuration: conf_data
  )

  custom_target('doc',
    build_by_default: true,
    command: [doxygen, doxyfile],
    depends: libsqsh,
    output: ['html']
  )
endif
