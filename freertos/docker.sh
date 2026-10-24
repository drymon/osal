#!/usr/bin/env bash
# Build and run dmosal's examples and cmocka tests on the FreeRTOS POSIX simulator, entirely
# inside Docker. The image (freertos/Dockerfile) contains the FreeRTOS kernel; only this repo
# is mounted. The first build creates the image.
#
#   freertos/docker.sh build   # build every examples/*.c and test/osal/*_test.c into build-freertos/
#   freertos/docker.sh run     # run the examples
#   freertos/docker.sh test    # run the cmocka tests (ctest)
#   freertos/docker.sh tidy    # clang-tidy over source/freertos/*.c
#   freertos/docker.sh shell   # interactive shell in the container
#
# CMAKE_ARGS=-DDYNAMIC_ALLOC=ON builds against the kernel's dynamic allocation API instead
# of static allocation (use BUILD_DIR=build-freertos-dyn to keep both).
# TEST=<name> limits test to one test (for example TEST=mutex_test).
set -euo pipefail

IMAGE=dmosal-freertos:local
BUILD=${BUILD_DIR:-build-freertos}
REPO="$(cd "$(dirname "$0")/.." && pwd)"

DOCKER_RUN=(docker run --rm -u "$(id -u):$(id -g)" -e HOME=/tmp -v "$REPO":/dmosal)

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
	docker build -t "$IMAGE" -f "$REPO/freertos/Dockerfile" "$REPO/freertos"
fi

case "${1:-}" in
build)
	"${DOCKER_RUN[@]}" "$IMAGE" sh -c \
		"cmake -S freertos/app -B $BUILD -G Ninja -DCMAKE_BUILD_TYPE=Debug ${CMAKE_ARGS:-} && cmake --build $BUILD"
	;;
run)
	for f in "$REPO"/examples/*.c; do
		ex="$(basename "$f" .c)"
		echo "=== $ex ==="
		"${DOCKER_RUN[@]}" "$IMAGE" timeout 20 "$BUILD/$ex"
	done
	;;
test)
	"${DOCKER_RUN[@]}" "$IMAGE" ctest --test-dir "$BUILD" --output-on-failure ${TEST:+-R "^$TEST\$"}
	;;
tidy)
	"${DOCKER_RUN[@]}" "$IMAGE" sh -c \
		"cmake -S freertos/app -B build-freertos-tidy -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null && cmake --build build-freertos-tidy >/dev/null && clang-tidy -p build-freertos-tidy --quiet --warnings-as-errors='*' source/freertos/*.c"
	;;
shell)
	"${DOCKER_RUN[@]}" -it "$IMAGE" bash
	;;
*)
	sed -n "2,11p" "$0"
	exit 1
	;;
esac
