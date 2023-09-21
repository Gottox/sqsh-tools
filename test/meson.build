add_languages('cpp', native: false)

testlib_dep = subproject('cextras').get_variable('testlib_dep')

subdir('libsqsh')

if get_option('test') == 'extended'
    subdir('tools')
endif
