#
# Makefile
# tox, 2018-04-02 16:39
#
include config.mk

HDR = \
	src/compression/buffer.h \
	src/compression/compression.h \
	src/context/compression_options_context.h \
	src/context/content_context.h \
	src/context/directory_context.h \
	src/context/fragment_context.h \
	src/context/inode_context.h \
	src/context/metablock_context.h \
	src/context/superblock_context.h \
	src/context/table_context.h \
	src/context/xattr_table_context.h \
	src/data/compression_options.h \
	src/data/compression_options_internal.h \
	src/data/datablock.h \
	src/data/datablock_internal.h \
	src/data/directory.h \
	src/data/directory_internal.h \
	src/data/fragment.h \
	src/data/fragment_internal.h \
	src/data/inode.h \
	src/data/inode_internal.h \
	src/data/metablock.h \
	src/data/metablock_internal.h \
	src/data/superblock.h \
	src/data/superblock_internal.h \
	src/data/xattr.h \
	src/data/xattr_internal.h \
	src/error.h \
	src/hsqs.h \
	src/mapper/memory_mapper.h \
	src/mapper/mmap.h \
	src/mapper/mmap_complete.h \
	src/mapper/static_memory.h \
	src/resolve_path.h \
	src/utils.h \
	src/utils/lru_hashmap.h \

SRC = \
	src/compression/buffer.c \
	src/compression/null.c \
	src/context/compression_options_context.c \
	src/context/content_context.c \
	src/context/directory_context.c \
	src/context/fragment_context.c \
	src/context/inode_context.c \
	src/context/metablock_context.c \
	src/context/superblock_context.c \
	src/context/table_context.c \
	src/context/xattr_table_context.c \
	src/data/compression_options.c \
	src/data/datablock.c \
	src/data/directory.c \
	src/data/fragment.c \
	src/data/inode.c \
	src/data/metablock.c \
	src/data/superblock.c \
	src/data/xattr.c \
	src/error.c \
	src/hsqs.c \
	src/mapper/memory_mapper.c \
	src/mapper/mmap.c \
	src/mapper/mmap_complete.c \
	src/mapper/static_memory.c \
	src/resolve_path.c \
	src/utils.c \
	src/utils/lru_hashmap.c \

LIBS = fuse3

OBJ = $(SRC:.c=.o)

BIN = \
	bin/hsqs-cat \
	bin/hsqs-ls \
	bin/hsqs-mount \
	bin/hsqs-info \

TST = \
	test/utils/lru_hashmap.c \
	test/integration.c \

TST_BIN = $(TST:.c=-test)

FZZ = \
	fuzzer/simple.c

FZZ_BIN = $(FZZ:.c=-fuzz)

BCH = \

BCH_BIN = $(BCH:.c=-bench)
BCH_CFLAGS = \

ifeq ($(CONFIG_COMPRESSION_GZIP), 1)
	SRC += 	src/compression/gzip.c
	CFLAGS += -DCONFIG_COMPRESSION_GZIP
	LIBS += zlib
endif
ifeq ($(CONFIG_COMPRESSION_LZ4), 1)
	SRC += 	src/compression/lz4.c
	CFLAGS += -DCONFIG_COMPRESSION_LZ4
	LIBS += liblz4
endif
ifeq ($(CONFIG_COMPRESSION_LZMA), 1)
	SRC += 	src/compression/lzma.c
	CFLAGS += -DCONFIG_COMPRESSION_LZMA
	LIBS += liblzma
endif
ifeq ($(CONFIG_COMPRESSION_LZO), 1)
	SRC += 	src/compression/lzo.c
	CFLAGS += -DCONFIG_COMPRESSION_LZO
	LIBS += lzo2
endif
ifeq ($(CONFIG_COMPRESSION_XZ), 1)
	SRC += 	src/compression/xz.c
	CFLAGS += -DCONFIG_COMPRESSION_XZ
	LIBS += liblzma
endif
ifeq ($(CONFIG_COMPRESSION_ZSTD), 1)
	SRC += 	src/compression/zstd.c
	CFLAGS += -DCONFIG_COMPRESSION_ZSTD
	LIBS += libzstd
endif

MAJOR=$(shell echo $(VERSION) | cut -d . -f 1)

CFLAGS += \
	$(shell pkg-config --cflags $(LIBS)) \

LDFLAGS += \
	$(shell pkg-config --libs $(LIBS)) \

all: $(BIN) libhsqs.a libhsqs.so

libhsqs.a: $(OBJ)
	@echo AR $@
	@ar rc $@ $(OBJ)
	@ranlib $@

libhsqs.so: $(OBJ)
	@echo SH $@
	@$(CC) -shared $(LDFLAGS) $(OBJ) -o $@.$(VERSION) -Wl,-soname=$@.$(MAJOR)
	@ln -sf $@.$(VERSION) $@.$(MAJOR)
	@ln -sf $@.$(VERSION) $@

bin/%: bin/%.o $(OBJ)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJ) $< -o $@

bench/%-bench: bench/%.c test/test.h $(OBJ)
	@echo CCBENCH $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(BCH_CFLAGS) $(OBJ) $< -o $@

test/%-test: test/%.c test/test.h $(SRC) gen/squash_image.h
	@echo CCTEST $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(TST_CFLAGS) $(SRC) $< -o $@

fuzzer/%-fuzz: fuzzer/%.c $(OBJ)
	@echo CCFUZZ $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $(FZZ_CFLAGS) $(SRC) $< -o $@

%.o: %.c $(HDR)
	@echo CC $@
	@$(CC) $(CFLAGS) -c -o $@ $<

check: $(TST_BIN)
	@for i in $(TST_BIN); do ./$$i || exit 1; done

speed: $(BCH_BIN) $(BENCH_JSON) $(BENCH_PLIST)
	@for i in $(BCH_BIN); do ./$$i; done

fuzz: $(FZZ_BIN)
	@for i in $(FZZ_BIN); do ./$$i $${i%-fuzz}-corpus -rss_limit_mb=0 -jobs=4 -max_total_time=600 -dict=$${i%-fuzz}-dict || exit 1; done

doc: doxygen.conf $(TST) $(SRC) $(HDR) README.md
	@sed -i "/^PROJECT_NUMBER\s/ s/=.*/= $(VERSION)/" $<
	@doxygen $<

coverage: check
	@mkdir -p cov
	@gcovr -r . --html --html-details -o cov/index.html

gen/squash_image.squashfs:
	mkdir -p $@_dir
	echo a > $@_dir/a
	seq 1 1050000 | tr -cd "\n" | tr '\n' b > $@_dir/b
	setfattr -n user.foo -v 1234567891234567891234567890001234567890 $@_dir/a
	setfattr -n user.bar -v 1234567891234567891234567890001234567890 $@_dir/b
	rm -f $@
	mksquashfs $@_dir $@ -nopad -force-uid 2020 -force-gid 202020 -Xcompression-level 1 -always-use-fragments
	rm -r $@_dir

gen/squash_image.h: gen/squash_image.squashfs
	printf "%s\n" "#include <stdint.h>" "static const uint8_t squash_image[] = {" > $@
	od  $< -t u1 -v -A n | tr ' ' '\n' | grep -v '^$$' | paste -sd, >> $@
	echo "};" >> $@

clean:
	@echo cleaning...
	@rm -rf doc cov gen
	@rm -f *.gcnp *.gcda
	@rm -f $(TST_BIN) $(BCH_BIN) $(FZZ_BIN) $(OBJ) $(BIN) libhsqs.so* libhsqs.a

.PHONY: check all clean speed coverage fuzz


# vim:ft=make
#
