subdir('collection')

subdir('concurrency')

src = [concurrency_src, collection_src]

cextras = static_library(
    'cextras',
    src,
    build_by_default: false,
    c_args: ['-fvisibility=hidden'],
)
