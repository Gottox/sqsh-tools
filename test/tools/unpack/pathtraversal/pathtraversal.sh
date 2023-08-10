#!/bin/sh

REFFILE="$SOURCE_ROOT/test/tools/ls/pathtraversal/pathtraversal.sqfs"
GOTCHA="/tmp/gotcha.txt"

if "$SQSH_UNPACK" "$REFFILE" / ""; then
	if [ -e "$GOTCHA" ]; then
		echo "Found $GOTCHA which should not be there"
		exit 1
	fi
fi
