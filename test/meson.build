add_languages('cpp', native: false)

utest_dep = dependency('utest')

subdir('libsqsh')

subdir('fuzzer')

if get_option('test') == 'extended'
    subdir('tools')
endif
