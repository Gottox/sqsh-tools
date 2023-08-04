######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : Makefile
# @created     : Sunday Dec 12, 2021 09:18:43 CET
######################################################################

NINJA_TARGETS := test benchmark install dist scan-build clang-format uninstall \
	all clang-tidy doc coverage-html

ifeq ($(32BIT),1)
	ARCH = i386
else
	ARCH = x86_64
endif

ifeq ($(MUON),1)
	MESON = muon
	NINJA = muon
else
	MESON = meson
	NINJA = ninja
endif
MESON_FLAGS += -Ddefault_library=static
MESON_FLAGS += -Dexamples=true
MESON_FLAGS += -Db_lundef=false
#MESON_FLAGS += -Dtest=extended
MESON_FLAGS += -Dtest=true
MESON_FLAGS += -Ddoc=internal
#MESON_FLAGS += -Dfuzzer=true
MESON_FLAGS += -Dfuzzer_timeout=10
MESON_FLAGS += -Dcurl=enabled
MESON_FLAGS += -Dzlib=enabled
MESON_FLAGS += -Dlz4=enabled
MESON_FLAGS += -Dlzma=enabled
MESON_FLAGS += -Dzstd=enabled
MESON_FLAGS += -Dfuse=enabled
MESON_FLAGS += -Dfuse-old=enabled
MESON_FLAGS += -Db_coverage=true

SANATIZE = 1

CC = gcc

ifeq ($(PODMAN), 1)
	W = podman run --rm -ti -v .:/host --device /dev/fuse --cap-add SYS_ADMIN gottox/sqsh-build:$(ARCH) \
		env
	BUILD_DIR = ./build-podman
else
	W =
	BUILD_DIR = ./build
	ifeq ($(SANATIZE), 1)
		MESON_FLAGS += -Db_sanitize=address,undefined
	endif
	MESON_FLAGS += -Dwerror=true
endif
ifeq ($(CC),clang)
	#MESON_FLAGS += -Dfuzzer=true
endif
ifeq ($(OPTIMIZE),1)
	MESON_FLAGS += --optimization=2
endif

.PHONY: $(NINJA_TARGETS)

$(NINJA_TARGETS): $(BUILD_DIR)
	$W $(NINJA) -C $< $@

$(BUILD_DIR): meson.build Makefile
	[ "$(PODMAN)" ] && meson wrap update-db || true
	[ -d "$@" ] && rm -rf "$@" || true
	$W CC=$(CC) $(MESON) setup $(MESON_FLAGS) "$@"

.PHONY: clean

clean:
	rm -rf "$(BUILD_DIR)"
