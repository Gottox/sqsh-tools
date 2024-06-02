fuzzer_repro = files(
    'crash_0c006fc7bbbab8ba82abca1f01cc1f95714aaf98',
    'crash_0f300b1e2028a520e74e06b9d598b97d15b34062',
    'crash_1938b2d283474afb40ccd2be49000bea75774c48',
    'crash_1c1af162b560e3a3f2dbd5b16db30265f2c4cc10',
    'crash_27e9bb6ee0b0d3a3bf838c52fe0de801cde1244b',
    'crash_28029d5df72ebdd793dcd3032fe12afa7ae8524b',
    'crash_2808240ac98ecfb63e0d9730e5ee3fb53682a52b',
    'crash_30b3ea0927886055d8817318bba33847a1a0d212',
    'crash_413a274deb971cdb406dac257e180dd935580505',
    'crash_4d490a2f06704698f75883f5f053946921b5d4cc',
    'crash_89fecf91d67a312d1ef19c559146b40cbfaa82d8',
    'crash_8ec28d2e3e155d341c61188291ad437bee2fc58d',
    'crash_912e7cee20e182e8f7f85ebd844bfa998b3800b3',
    'crash_a45a78dd8471a4fb501d65557641fdc3ba270a3c',
    'crash_c4192decccf07b476e5d200d40f180f042ef8380',
    'crash_cadde0fee8feaf097549ef550a5d5967393945df',
    'crash_d636c46db3865f401b3e49e9ba1f95327576b8e2',
    'crash_e28e4256647bd3a59110dfa8bd20e73f8f316fae',
    'crash_e4d6f2e2cc5db10a41af1d2a271778825cb24ef4',
    'crash_e6ffcaad48c4a1ecad7cae0cc3a0776e2fa356c8',
    'crash_e770c992b26a129aaeea42993b2a9b06ba91faca',
    'crash_f7bc33d88bbc6a270c32bc6969059a0c44c49dd4',
    'leak_2f97e04d5e7167856ba0827c10939690d203cb54',
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
