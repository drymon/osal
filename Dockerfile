# Pinned build/test environment for dmosal.
#
# This image locks the exact toolchain package versions used to build, test,
# lint, and document the library, so `docker build` reproduces the same
# environment on any machine (and mirrors what CI resolves on ubuntu-22.04).
#
# Usage:
#   docker build -t dmosal-dev .
#   docker run --rm -it -v "$PWD":/workspace -w /workspace dmosal-dev bash
#   # or, without an interactive shell:
#   docker run --rm -v "$PWD":/workspace -w /workspace dmosal-dev just check
#
# To refresh pins (e.g. after a new Ubuntu 22.04 security update), run:
#   docker run --rm ubuntu:22.04 bash -c \
#     "apt-get update -qq && apt-cache policy <pkg1> <pkg2> ..."
# and update the versions below accordingly.

FROM ubuntu:22.04

ARG CMAKE_VERSION=3.22.1-1ubuntu1.22.04.2
ARG CMOCKA_VERSION=1.1.5-2
ARG DOXYGEN_VERSION=1.9.1-2ubuntu2
ARG GRAPHVIZ_VERSION=2.42.2-6ubuntu0.1
ARG CLANG_FORMAT_VERSION=1:14.0-55~exp2
ARG CLANG_TIDY_VERSION=1:14.0-55~exp2
ARG VALGRIND_VERSION=1:3.18.1-1ubuntu2
ARG LCOV_VERSION=1.15-1
ARG CPPCHECK_VERSION=2.7-1
ARG GCC_VERSION=4:11.2.0-1ubuntu1
ARG GXX_VERSION=4:11.2.0-1ubuntu1
ARG CLANG_VERSION=1:14.0-55~exp2

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates curl \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends \
      cmake=${CMAKE_VERSION} \
      libcmocka-dev=${CMOCKA_VERSION} \
      doxygen=${DOXYGEN_VERSION} \
      graphviz=${GRAPHVIZ_VERSION} \
      clang-format=${CLANG_FORMAT_VERSION} \
      clang-tidy=${CLANG_TIDY_VERSION} \
      valgrind=${VALGRIND_VERSION} \
      lcov=${LCOV_VERSION} \
      cppcheck=${CPPCHECK_VERSION} \
      gcc=${GCC_VERSION} \
      g++=${GXX_VERSION} \
      clang=${CLANG_VERSION} \
      make \
      git \
      python3-pip \
    && rm -rf /var/lib/apt/lists/*

# just: pinned via its official install script + explicit tag, not apt
# (not packaged in the jammy repos).
ARG JUST_VERSION=1.36.0
RUN curl --proto '=https' --tlsv1.2 -sSf \
      https://just.systems/install.sh \
    | bash -s -- --tag ${JUST_VERSION} --to /usr/local/bin

# pre-commit: pinned exact version via pip.
ARG PRE_COMMIT_VERSION=3.7.1
RUN pip3 install --no-cache-dir "pre-commit==${PRE_COMMIT_VERSION}"

WORKDIR /workspace
