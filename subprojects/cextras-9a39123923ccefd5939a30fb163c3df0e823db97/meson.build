project(
    'cextras',
    'c',
    default_options: [
        'c_std=c11',
        'warning_level=3',
        'optimization=0',
    ],
    version: '0.1',
)

threads_dep = dependency('threads', required: get_option('threads'))

subdir('lib')
subdir('include')

cextras_dep = declare_dependency(
    include_directories: include_directories('include'),
    link_with: cextras,
    dependencies: threads_dep,
)

subdir('testlib')

if get_option('test')
    subdir('tests')
endif
