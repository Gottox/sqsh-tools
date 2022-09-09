FROM alpine:3.16

RUN apk add --no-cache gcc clang clang-analyzer meson musl-dev git
RUN apk add --no-cache curl-dev zlib-dev lz4-dev xz-dev lzo-dev zstd-dev \
	fuse3-dev squashfs-tools attr doxygen graphviz

WORKDIR /host
