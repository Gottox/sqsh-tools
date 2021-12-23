#!/bin/sh

CC=clang meson setup /tmp/build \
  -Dcurl=false \
  -Dzlib=true \
  -Dlz4=false\
  -Dlzma=false \
  -Dlzo2=false \
  -Dzstd=false \
  -Dtest=true
exec ninja -C /tmp/build test
