ARG BASE_IMAGE=alpine
#ARG BASE_IMAGE=i386/alpine
FROM ${BASE_IMAGE}:3.20

RUN apk add --no-cache \
	gcc \
	g++ \
	clang17 \
	clang17-extra-tools \
	clang17-analyzer \
	meson \
	musl-dev \
	git \
	compiler-rt
RUN apk add --no-cache \
	curl-dev \
	fuse3-dev \
	fuse-dev \
	zlib-dev \
	lz4-dev \
	xz-dev \
	zstd-dev \
	squashfs-tools \
	attr \
	doxygen \
	graphviz \
	valgrind
RUN apk add --no-cache \
	--repository https://dl-cdn.alpinelinux.org/alpine/edge/main \
	--repository https://dl-cdn.alpinelinux.org/alpine/edge/testing \
	pkgconf \
	muon

WORKDIR /host
