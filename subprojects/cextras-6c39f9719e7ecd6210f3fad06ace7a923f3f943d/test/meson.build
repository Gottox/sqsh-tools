add_languages('cpp', native: false)

tests = [
    'testlib.c',
    'testlib.cpp',
    'concurrency/threadpool_test.c',
    'concurrency/future_test.c',
    'collection/buffer_test.c',
    'collection/collector.c',
    'collection/lru_test.c',
    'collection/rc_hash_map_test.c',
    'collection/rc_map_test.c',
    'memory/rc.c',
    'memory/utils.c',
]

foreach test : tests
    test_exe = executable(
        test.underscorify(),
        test,
        install: false,
        dependencies: [
            testlib_dep,
            cextras_dep,
        ],
    )
    test(test, test_exe)
endforeach
