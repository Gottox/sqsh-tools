markdown = files(
    '../../README.md',
    '../../doc/guide.md',
    '../../libsqsh/README.md',
)

foreach m : markdown
    name = m.full_path().underscorify()
    src = custom_target(
        name + '.c',
        input: m,
        output: name + '.c',
        command: [
            md2code,
            '-i', 'sqsh.h',
            '-i', 'assert.h',
            '-i', 'stdlib.h',
            '@INPUT@',
            '@OUTPUT@',
        ],
    )
    exe = executable(
        name,
        sources: [src],
        dependencies: [libsqsh_dep],
        build_by_default: true,
    )
endforeach
