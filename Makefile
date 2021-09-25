#
# Makefile
# tox, 2018-04-02 16:39
#
include config.mk

HDR = \
	src/compression/buffer.h \
	src/compression/compression.h \
	src/context/datablock_context.h \
	src/context/directory_context.h \
	src/context/fragment_context.h \
	src/context/inode_context.h \
	src/context/metablock_context.h \
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
	src/error.h \
	src/resolve_path.h \
	src/squash.h \
	src/utils.h \

SRC = \
	src/compression/buffer.c \
	src/compression/null.c \
	src/context/datablock_context.c \
	src/context/directory_context.c \
	src/context/fragment_context.c \
	src/context/inode_context.c \
	src/context/metablock_context.c \
	src/data/compression_options.c \
	src/data/datablock.c \
	src/data/directory.c \
	src/data/fragment.c \
	src/data/inode.c \
	src/data/metablock.c \
	src/data/superblock.c \
	src/error.c \
	src/resolve_path.c \
	src/squash.c \
	src/utils.c \

LIBS = fuse3

OBJ = $(SRC:.c=.o)

BIN = \
	bin/catsquash \
	bin/lssquash \
	bin/squashinfo \
	bin/squashfs-fuse \

TST = \

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

all: $(BIN) libsquashfs.a libsquashfs.so

libsquashfs.a: $(OBJ)
	@echo AR $@
	@ar rc $@ $(OBJ)
	@ranlib $@

libsquashfs.so: $(OBJ)
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

test/%-test: test/%.c test/test.h $(SRC)
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
	@for i in $(FZZ_BIN); do ./$$i $${i%-fuzz}-corpus -max_total_time=120 -dict=$${i%-fuzz}-dict || exit 1; done

doc: doxygen.conf $(TST) $(SRC) $(HDR) README.md
	@sed -i "/^PROJECT_NUMBER\s/ s/=.*/= $(VERSION)/" $<
	@doxygen $<

bench/bench-file.json:
	@wget http://eu.battle.net/auction-data/258993a3c6b974ef3e6f22ea6f822720/auctions.json -O $@

bench/bench-file.plist: bench/bench-file.json
	@npm install plist-cli
	@node_modules/.bin/plist-cli < $< > $@
	@rm -rf package-lock.json node_modules

coverage: check
	@mkdir cov
	@gcovr -r . --html --html-details -o cov/index.html

clean:
	@echo cleaning...
	@rm -rf doc cov
	@rm -f *.gcnp *.gcda
	@rm -f $(TST_BIN) $(BCH_BIN) $(FZZ_BIN) $(OBJ) $(BIN) libsquashfs.so* libsquashfs.a

.PHONY: check all clean speed coverage fuzz


# vim:ft=make
#
