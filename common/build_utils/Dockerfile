ARG BASE_IMAGE=alpine
#ARG BASE_IMAGE=i386/alpine
FROM docker.io/${BASE_IMAGE}:3.21

RUN apk add --no-cache \
	jq \
	gcc \
	g++ \
	clang19 \
	clang19-extra-tools \
	clang19-analyzer \
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
	--repository https://dl-cdn.alpinelinux.org/alpine/edge/community \
	pkgconf \
	muon

WORKDIR /host
