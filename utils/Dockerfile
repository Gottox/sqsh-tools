# This is a Dockerfile that is used for running unit tests.
FROM voidlinux/voidlinux

RUN xbps-install -Syu xbps && \
	xbps-install -yu && \
	xbps-install -y base-devel clang libsanitizer-devel zlib-devel liblzma-devel lzo-devel liblz4-devel libzstd-devel fuse3-devel squashfs-tools && \
	rm -r /var/cache/xbps
