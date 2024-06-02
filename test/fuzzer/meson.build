fuzzer_repro = files(
    'fuzz_crash_1',
    'fuzz_crash_10',
    'fuzz_crash_11',
    'fuzz_crash_12',
    'fuzz_crash_13',
    'fuzz_crash_14',
    'fuzz_crash_15',
    'fuzz_crash_16',
    'fuzz_crash_17',
    'fuzz_crash_18',
    'fuzz_crash_19',
    'fuzz_crash_2',
    'fuzz_crash_20',
    'fuzz_crash_21',
    'fuzz_crash_22',
    'fuzz_crash_3',
    'fuzz_crash_4',
    'fuzz_crash_5',
    'fuzz_crash_6',
    'fuzz_crash_7',
    'fuzz_crash_8',
    'fuzz_crash_9',
    'fuzz_leak_1',
)

utest_dep = dependency('utest')

fuzzer_repro_codegen = executable('repro_codegen', 'repro_codegen.c')

fuzzer_repro_tgt = custom_target(
    'repro_test',
    output: 'repro_test.c',
    capture: true,
    input: fuzzer_repro,
    command: [
        fuzzer_repro_codegen,
        '@INPUT@',
    ],
)

fuzzer_src = files(
    '../../fuzzer/simple.c',
)
fuzzer_repro_test = executable(
    'repro_test',
    [fuzzer_repro_tgt, fuzzer_src],
    include_directories: [
        libsqsh_include,
        libsqsh_common_include,
        libsqsh_private_include,
        libmksqsh_include,
    ],
    link_with: [libsqsh.get_static_lib(), libmksqsh.get_static_lib()],
    dependencies: [threads_dep, utest_dep, cextras_dep],

)

test('simple_fuzzer_repro', fuzzer_repro_test)
