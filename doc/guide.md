@page working_with_archives Working with Archives

@tableofcontents

# Introduction

The @ref SqshArchive struct represents a squashfs archive. It is
created by calling
@ref SqshArchive::sqsh_archive_open "sqsh_archive_open()" and is destroyed by calling
@ref SqshArchive::sqsh_archive_close "sqsh_archive_close()". 

Opening and cleaning up an archive is simple:
```c
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs", NULL, NULL);
assert(archive != NULL);

sqsh_archive_close(archive);
```

Opening with more control:
```c
int err = 0;
struct SqshConfig config = {
	.max_symlink_depth = 10,
	// ...
};
struct SqshArchive *archive = sqsh_archive_open("archive.sqsh", &config, &err);
if (err != 0) {
	sqsh_perror(err, NULL);
	exit(1);
}

sqsh_archive_close(archive);
```

# Reading files in the archive

## Simple

The simplest way to read a file in the archive is to use
@ref sqsh_easy_file_content "sqsh_easy_file_content()". This function 
returns a pointer to the contents of the file in the archive. The pointer 
is heap allocated and must be free'd by the caller. The allocated buffer is
null terminated. If the file itself contains null bytes, you can get the 
file size by calling @ref sqsh_easy_file_size "sqsh_easy_file_size()".

```c
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs", NULL, NULL);
assert(archive != NULL);

uint8_t *content = sqsh_easy_file_content(archive, "/path/to/file.txt", NULL);
size_t size = sqsh_easy_file_size(archive, "/path/to/file.txt", NULL);
fwrite(content, 1, size, stdout);
free(content);

sqsh_archive_close(archive);
```

## Advanced: File Iterators

The simple API is fine for small files, but for larger files it is better
to use the @ref SqshFileIterator. The iterator allows you to read the file
in chunks, which is more efficient than reading the entire file into memory
at once.

```c
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs", NULL, NULL);

int err = 0;
struct SqshFile *file = sqsh_open(archive, "/path/to/file.txt", &err);
assert(err == 0);
struct SqshFileIterator *it = sqsh_file_iterator_new(file, &err);
assert(err == 0);
while ((err = sqsh_file_iterator_next(it, SIZE_MAX, &err)) > 0) {
	const uint8_t *data = sqsh_file_iterator_data(it);
	size_t size = sqsh_file_iterator_size(it);
	fwrite(data, 1, size, stdout);
}
assert(err == 0);
sqsh_file_iterator_free(it);
sqsh_close(file);
sqsh_archive_close(archive);
```

## Advanced: File Reader

If you need more control over the ranges to be read from the file, you can use 
the @ref SqshFileReader API. This API allows you to read arbitrary ranges from
the file.

```c 
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs", NULL, NULL);

int err = 0;
struct SqshFile *file = sqsh_open(archive, "/path/to/file.txt", &err);
assert(err == 0);
struct SqshFileReader *reader = sqsh_file_reader_new(file, &err);
assert(err == 0);
err = sqsh_file_reader_advance(reader, 0, 10);
assert(err == 0);

const uint8_t *data = sqsh_file_reader_data(reader);
size_t size = sqsh_file_reader_size(reader);

fwrite(data, 1, size, stdout);
sqsh_file_reader_free(reader);
sqsh_close(file);
sqsh_archive_close(archive);
```

This example reads the first 10 bytes of the file.

# Reading xattrs

`libsqsh` is able to handle handle xattrs of files in the archive.

## Simple

The simple way to get an xattr is to use @ref sqsh_easy_xattr_get "sqsh_easy_xattr_get()".

```c
int err = 0;
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs", NULL, NULL);
assert(err == 0);
char *xattr = sqsh_easy_xattr_get(archive, "/path/to/file.txt", "user.myxattr", &err);
assert(err == 0);
puts(xattr);
free(xattr);
sqsh_archive_close(archive);
```

To get a list of all xattr on a file you can use sqsh_easy_xattr_list "sqsh_easy_xattr_list()".

```c
int err = 0;
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs", NULL, NULL);
assert(err == 0);
char **list = sqsh_easy_xattr_keys(archive, "/path/to/file.txt", &err);
assert(err == 0);
for(int i = 0; list[i] != NULL; i++) {
	puts(list[i]);
}
free(list);
sqsh_archive_close(archive);
```
