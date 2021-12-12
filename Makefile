######################################################################
# @author      : Enno Boland (mail@eboland.de)
# @file        : Makefile
# @created     : Sunday Dec 12, 2021 09:18:43 CET
######################################################################

NINJA_TARGETS = all compile test clean

.PHONY: $(NINJA_TARGETS)

$(NINJA_TARGETS): builddir
	ninja -C $< $@

builddir:
	meson setup builddir

.PHONY: meson_clean

meson_clean:
	rm -r builddir
