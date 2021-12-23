######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : Makefile
# @created     : Sunday Dec 12, 2021 09:18:43 CET
######################################################################

NINJA_TARGETS := test benchmark install dist scan-build clang-format uninstall \
	all tidy

ifeq ($(PODMAN), 1)
	W = podman run --rm -ti -v .:/host gottox/hsqs-build
	SANATIZE =
	BUILD_DIR = ./build_dir-podman
else
	W =
	SANATIZE = -Db_sanitize=address,undefined
	BUILD_DIR = ./build_dir
endif

.PHONY: $(NINJA_TARGETS)

$(NINJA_TARGETS): $(BUILD_DIR)
	$W ninja -C $< $@

$(BUILD_DIR): meson.build Makefile
	[ -d "$@" ] && rm -r "$@" || true
	$W meson setup "$@" -Dtest=true $(SANATIZE)

.PHONY: clean

clean:
	[ -d "$(BUILD_DIR)" ] && rm -r $(BUILD_DIR) || true
