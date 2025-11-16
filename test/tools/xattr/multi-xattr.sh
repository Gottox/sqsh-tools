#!/bin/sh -ex

case "$(uname -s)" in
	# openbsd or macos do not support extended attributes in squashfs
	OpenBSD|Darwin)
		echo "Skipping test on $(uname -s)"
		;;
esac

: "${BUILD_DIR:?BUILD_DIR is not set}"
: "${MKSQUASHFS:?MKSQUASHFS is not set}"
: "${SOURCE_ROOT:?SOURCE_ROOT is not set}"
: "${SQSH_XATTR:?SQSH_XATTR is not set}"

MKSQUASHFS_OPTS="-noappend -all-root -mkfs-time 0"

WORK_DIR="$BUILD_DIR/xattr"

mkdir -p "$WORK_DIR/root"
cd "$WORK_DIR"

echo "payload" > "$WORK_DIR/root/file"

cat > "$WORK_DIR/xattr.pseudo" <<'EOF'
/file x user.alpha=hello
/file x trusted.beta=bin\001\002"\trail
EOF

# shellcheck disable=SC2086
"$MKSQUASHFS" "$WORK_DIR/root" "$WORK_DIR/xattr.squashfs" -pf "$WORK_DIR/xattr.pseudo" $MKSQUASHFS_OPTS

expected_output=$(cat <<'EOT' | sort
trusted.beta="bin\001\002"\trail"
user.alpha="hello"
EOT
)

result=$("$SQSH_XATTR" "$WORK_DIR/xattr.squashfs" /file | sort)
[ "$result" = "$expected_output" ]
