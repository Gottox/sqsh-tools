ARG BASE_IMAGE=alpine
#ARG BASE_IMAGE=i386/alpine
FROM ${BASE_IMAGE}:3.18

RUN apk add --no-cache gcc g++ clang16 clang16-analyzer meson musl-dev git \
	compiler-rt
RUN apk add --no-cache curl-dev fuse fuse3-dev fuse-dev squashfs-tools attr doxygen \
	graphviz valgrind

WORKDIR /host
