fuzzer_src = ['simple.c']

fuzzer_c_args = [
    '-fsanitize=fuzzer-no-link',
    '-fprofile-instr-generate',
    '-fcoverage-mapping',
]

fuzzer_link_args = ['-fsanitize=fuzzer']
source_corpus = join_paths(meson.current_source_dir(), 'corpus')

mkdir = find_program('mkdir')

corpus_tgt = custom_target(
    'corpus',
    output: 'corpus',
    command: [mkdir, '-p', '@OUTPUT@'],
)

fuzzer = executable(
    'simple_fuzzer',
    fuzzer_src,
    c_args: fuzzer_c_args,
    link_args: fuzzer_link_args,
    dependencies: cextras_dep,
    include_directories: [
        libsqsh_include,
        libsqsh_common_include,
        libsqsh_private_include,
    ],
    link_with: libsqsh.get_static_lib(),
)

rundir_tgt = custom_target(
    'fuzzer_inputs',
    output: 'inputs',
    command: [mkdir, '-p', '@OUTPUT@'],
)

test(
    'simple_fuzzer',
    fuzzer,
    workdir: rundir_tgt.full_path(),
    is_parallel: false,
    depends: [corpus_tgt, rundir_tgt],
    args: [
        corpus_tgt.full_path(),
        source_corpus,
        '-error_exitcode=1',
        '-timeout_exitcode=1',
        '-timeout=10',
        '-rss_limit_mb=0',
        '-malloc_limit_mb=10',
        '-max_total_time=@0@'.format(get_option('fuzzer_timeout')),
    ],
    timeout: 10 + get_option('fuzzer_timeout'),
)
