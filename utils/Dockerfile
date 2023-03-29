ARG BASE_IMAGE=alpine
#ARG BASE_IMAGE=i386/alpine
FROM ${BASE_IMAGE}:3.17

RUN apk add --no-cache gcc g++ clang15 clang15-analyzer meson musl-dev git \
	compiler-rt
RUN apk add --no-cache curl-dev zlib-dev lz4-dev xz-dev lzo-dev zstd-dev \
	fuse fuse3-dev squashfs-tools attr doxygen graphviz

WORKDIR /host
