#
# liblifthttpbuilder Dockerfile
#
# Based on:
# https://hub.docker.com/_/fedora
#
# Documentation:
# https://github.com/jbaldwin/liblifthttp

# Pull base image.
FROM fedora:latest

# Install build dependencies.
RUN dnf update -y
RUN dnf upgrade -y
RUN dnf install -y \
    git \
    cmake \
    ninja-build \
    g++ \
    clang \
    zlib-devel \
    libcurl-devel \
    libuv-devel

COPY docker-entrypoint.d/docker-entrypoint.sh /

# Define default command.
ENTRYPOINT ["/docker-entrypoint.sh", "/bin/bash"]