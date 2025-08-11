add_languages('cpp', native: false)

subdir('libsqsh')

subdir('fuzzer')

# https://github.com/Gottox/md2code
md2code = find_program('md2code', required: false)
if md2code.found()
    subdir('doctest')
endif

if get_option('test') == 'extended'
    subdir('tools')
endif
