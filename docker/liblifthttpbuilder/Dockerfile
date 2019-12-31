#
# liblifthttpbuilder Dockerfile
#
# Based on:
# https://github.com/dockerfile/ubuntu
#
# Documentation:
# https://github.com/jbaldwin/liblifthttp

# Pull base image.
FROM ubuntu:latest

# Install build dependencies.
RUN \
  apt-get update && \
  apt-get -y upgrade && \
  apt-get install -y \
    git \
    cmake \
    ninja-build \
    g++ \
    clang \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libuv1-dev

COPY docker-entrypoint.d/docker-entrypoint.sh /

# Define default command.
ENTRYPOINT ["/docker-entrypoint.sh", "tail -f /dev/null"]