fuzzer_src = ['simple.c']

fuzzer_c_args = ['-fsanitize=fuzzer-no-link', '-fprofile-instr-generate', '-fcoverage-mapping']

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
    dependencies: libsqsh_dependency,
)

test(
    'simple_fuzzer',
    fuzzer,
    args: [corpus_tgt.full_path(), source_corpus,
        '-max_total_time=10',
        '-error_exitcode=1',
        '-timeout_exitcode=1',
    ],
    depends: corpus_tgt,
)
