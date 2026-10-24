# Slim build environment for running dmosal on the FreeRTOS POSIX/Linux simulator.
#
# The simulator port compiles FreeRTOS + the app into a Linux executable with the host gcc, so
# no cross toolchain or emulator is needed.
#
# Built and used through freertos/docker.sh; see that script for usage.
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential ca-certificates cmake curl libcmocka-dev ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Pinned FreeRTOS kernel, downloaded once into the image (no host download).
ARG FREERTOS_KERNEL_VERSION=V11.1.0
ENV FREERTOS_KERNEL_PATH=/opt/FreeRTOS-Kernel
RUN mkdir -p ${FREERTOS_KERNEL_PATH} \
    && curl -fsSL https://github.com/FreeRTOS/FreeRTOS-Kernel/archive/refs/tags/${FREERTOS_KERNEL_VERSION}.tar.gz \
       | tar -xz -C ${FREERTOS_KERNEL_PATH} --strip-components=1 \
    && chmod -R a+rX ${FREERTOS_KERNEL_PATH}

WORKDIR /dmosal
