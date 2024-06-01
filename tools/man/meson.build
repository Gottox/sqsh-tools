tools_manpages = [
    'sqsh-cat.1',
    'sqsh-ls.1',
    'sqsh-stat.1',
    'sqsh-unpack.1',
    'sqsh-xattr.1',
]

conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())

if fuse3_dep.found() or fuse2_dep.found()
    tools_manpages += ['sqshfs.1']
endif

foreach template : tools_manpages
    manpage = configure_file(
        input: template + '.in',
        output: template,
        configuration: conf_data,
    )
    install_man(manpage)
endforeach
