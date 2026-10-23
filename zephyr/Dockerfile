# Slim build environment for running dmosal on Zephyr's native_sim.
#
# native_sim compiles Zephyr + the app into a Linux executable with the host
# gcc, so no Zephyr SDK / cross toolchains are needed. Add the SDK (or switch
# to ghcr.io/zephyrproject-rtos/ci) when targeting QEMU boards.
#
# Built and used through zephyr/docker.sh; see that script for usage.
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive \
    ZEPHYR_TOOLCHAIN_VARIANT=host \
    PATH=/opt/venv/bin:$PATH

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential ca-certificates cmake device-tree-compiler git gperf \
        ninja-build python3 python3-venv curl xz-utils \
    && rm -rf /var/lib/apt/lists/* \
    && python3 -m venv /opt/venv \
    && pip install --no-cache-dir west

# Zephyr source lives in the image (no host download). Only the zephyr repo is
# fetched; native_sim needs none of the optional HAL modules.
ARG ZEPHYR_VERSION=v4.0.0
ENV ZEPHYR_BASE=/opt/zephyrws/zephyr
RUN west init --mr ${ZEPHYR_VERSION} /opt/zephyrws \
    && cd /opt/zephyrws/zephyr && git gc --prune=all -q \
    && pip install --no-cache-dir -r scripts/requirements-base.txt \
    && chmod -R a+rX /opt/zephyrws

# cmocka (Zephyr 4.0 ships no cmocka module): the tests in test/osal are built against its
# sources by zephyr/app.
ARG CMOCKA_VERSION=1.1.7
RUN mkdir -p /opt/cmocka \
    && curl -fsSL https://cmocka.org/files/1.1/cmocka-${CMOCKA_VERSION}.tar.xz \
       | tar -xJ -C /opt/cmocka --strip-components=1
# west and cmake write caches inside the Zephyr tree, and the container runs as the host user.
RUN mkdir -p /opt/zephyrws/zephyr/.cache && chmod -R a+rwX /opt/zephyrws/.west /opt/zephyrws/zephyr/.cache

WORKDIR /dmosal
