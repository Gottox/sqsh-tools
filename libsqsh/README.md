# libsqsh

*libsqsh* is a purely 2-Clause BSD Licensed implementation of the squashfs
filesystem in C11. It covers the complete squashfs feature set, while still having
minimal memory footprint.

Note that *libsqsh* only supports reading squashfs archives. If you want to create
squashfs archives, you can either use
[squashfs-tools](https://github.com/plougher/squashfs-tools/), which provides a
command line interface, or
[squashfs-tools-ng](https://github.com/AgentD/squashfs-tools-ng/), which provides both
a command-line interface and a C library interface.

## Features

* **Complete feature set**: libsqsh supports all features of the squashfs file
  system.

* **Thread-safe**: libsqsh is thread-safe and can be used in multi-threaded
  applications.

* **Safe**: While we cannot guarantee that libsqsh is bug-free, we have a test
  suite that covers lots of edge cases and fuzz the library regularly. We trust it
  enough to use libsqsh to open untrusted squashfs archives. libsqsh runs consistency 
  checks over the squashfs archive to catch loops in the directory trees, invalid file
  names and a few other error states that are not covered by other implementations.

* **Fast**: while we need to keep a trade off between safety and speed, we try to
  keep the performance impact as low as possible. In
  [certain scenarios](https://chaos.social/@Gottox/112571790117451071), libsqsh
  ranks among the fastest squashfs implementations.

* **Modern C11 implementation**: Written in modern C11, libsqsh is designed for
  efficiency and compatibility with C++, C, and other languages.

* **Pure 2-Clause-BSD License**: libsqsh is licensed under the 2-Clause BSD
  license. This allows you to use libsqsh in any project, even commercial ones.

## Example

This is a simple example that a) prints the content of a file and b) lists the
content of a directory.

```c
struct SqshArchive *archive =
		sqsh_archive_open("/path/to/archive.squashfs", NULL, NULL);
assert(archive != NULL);

uint8_t *contents = sqsh_easy_file_content(archive, "/path/to/file", NULL);
assert(contents != NULL);
const size_t size = sqsh_easy_file_size(archive, "/path/to/file", NULL);
fwrite(contents, 1, size, stdout);
free(contents);

char **files = sqsh_easy_directory_list(archive, "/path/to/dir", NULL);
assert(files != NULL);
for (int i = 0; files[i] != NULL; i++) {
	puts(files[i]);
}
free(files);

sqsh_archive_close(archive);
```

Find further examples in the [examples](../examples) directory.

## Note on Deprecation

The sqsh-tools team works hard to keep the API stable. As a result,
some functions are marked as deprecated but remain available for the
current major version. The replacements for these deprecated functions
typically have the same name with a `...2` suffix.

In the next major (at the time of writing v2.0.0) version, the `...2`
functions will be renamed to their original names, and the deprecated
functions will be removed.

# Resource

* https://dr-emann.github.io/squashfs/squashfs.html - A detailed description of
  the squashfs file format.
* https://github.com/plougher/squashfs-tools/ - The original squashfs-tools
  implementation.
* https://github.com/AgentD/squashfs-tools-ng/ - A rewrite of the squashfs-tools
  implementation with library support. In contrast to libsqsh, this library is
  able to write squashfs archives.
* https://github.com/vasi/squashfuse - A C implementation of the squashfs
  filesystem.
* https://github.com/wcampbell0x2a/backhand - A Rust implementation of the
  squashfs filesystem.

