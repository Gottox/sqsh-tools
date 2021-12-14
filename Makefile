######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : Makefile
# @created     : Sunday Dec 12, 2021 09:18:43 CET
######################################################################

NINJA_TARGETS := test benchmark install dist scan-build clang-format uninstall \
	all clean

.PHONY: $(NINJA_TARGETS)

$(NINJA_TARGETS): builddir
	ninja -C $< $@

builddir: meson.build
	[ -d "builddir" ] && rm -r builddir || true
	meson setup builddir -Dtest=true

.PHONY: meson_clean

meson_clean:
	rm -r builddir
