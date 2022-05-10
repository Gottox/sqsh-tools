# podman build -t docker.io/gottox/hsqs-build .
# podman push docker.io/gottox/hsqs-build

FROM alpine:3.15

RUN apk add --no-cache gcc clang clang-analyzer meson musl-dev git
RUN apk add --no-cache curl-dev zlib-dev lz4-dev xz-dev lzo-dev zstd-dev \
	fuse3-dev squashfs-tools attr doxygen graphviz

WORKDIR /host
