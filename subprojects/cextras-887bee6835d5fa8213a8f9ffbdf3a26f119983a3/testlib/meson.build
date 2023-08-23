testlib_src = ['driver.c']

testlib = static_library('testlib', testlib_src, build_by_default: false)

testlib_dep = declare_dependency(
    link_with: testlib,
    include_directories: ['include'],
)
