@page working_with_archives Working with Archives

@tableofcontents

# Introduction

The @ref SqshArchive struct represents a squashfs archive. It is
created by calling
@ref SqshArchive::sqsh_archive_open "sqsh_archive_open()" and is destroyed by calling
@ref SqshArchive::sqsh_archive_close "sqsh_archive_close()". If you need better control
over the archive creation you can use the 
@ref SqshArchive::sqsh_archive_new "sqsh_archive_new()" function, which accepts
a @ref SqshConfig struct that gives you more control over the archive and better
error reporting.

Opening:
```c 
struct SqshArchive *archive = sqsh_archive_open("archive.squashfs");
```

Closing:
```c 
sqsh_archive_close(archive);
```

Opening with more control:
```c 
int err = 0;
struct SqshConfig config = {

};
struct SqshArchive *archive = sqsh_archive_new("archive.sqsh", &config, &err);
if (err != 0) {
	sqsh_perror(err);
}
```

# Reading files in the archive

## Simple

The simplest way to read a file in the archive is to use
@ref sqsh_file_content "sqsh_file_content()". This function 
returns a pointer to the contents of the file in the archive. The pointer 
is heap allocated and must be freed by the caller. The allocated buffer is
null terminated. If the file itself contains null bytes, you can get the 
file size by calling @ref sqsh_file_size "sqsh_file_size()".

```c
char *content = sqsh_file_content(archive, "/path/to/file.txt");
size_t size = sqsh_file_size(archive, "/path/to/file.txt");
fwrite(content, 1, size, stdout);
free(content);
```

## Advanced: File Iterators

The simple API is fine for small files, but for larger files it is better
to use the @ref SqshFileIterator. The iterator allows you to read the file
in chunks, which is more efficient than reading the entire file into memory
at once.

```c
int err = 0;
struct SqshInode *inode = sqsh_open(archive, "/path/to/file.txt", &err);
if (err != 0) {
	sqsh_perror(err);
	return;
}
struct SqshFileIterator *it = sqsh_file_iterator_new(archive, "/path/to/file.txt", &err);
if (err != 0) {
	sqsh_perror(err);
	return;
}
while ((err = sqsh_file_iterator_next(it)) > 0) {
	const uint8_t *data = sqsh_file_iterator_data(it);
	size_t size = sqsh_file_iterator_size(it);
	fwrite(data, 1, size, stdout);
}
if (err != 0) {
	sqsh_perror(err);
	return;
}
sqsh_file_iterator_free(it);
```

## Advanced: File Reader

If you need more control over the ranges to be read from the file, you can use 
the @ref SqshFileReader API. This API allows you to read arbitrary ranges from
the file.

```c 
int err = 0;
struct SqshFileReader *reader = sqsh_file_reader_new(archive, "/path/to/file.txt", &err);
if (err != 0) {
	sqsh_perror(err);
	return;
}
err = sqsh_file_reader_advance(reader, 0, 10);
if (err != 0) {
	sqsh_perror(err);
	return;
}
const uint8_t *data = sqsh_file_reader_data(reader);
size_t size = sqsh_file_reader_size(reader);
fwrite(data, 1, size, stdout);

sqsh_file_reader_free(reader);
```

This example reads the first 10 bytes of the file.

