#!/usr/bin/env bash
# Build and run dmosal's Zephyr examples on native_sim, entirely inside Docker.
# The image (zephyr/Dockerfile) contains Zephyr; only this repo is mounted.
# The first build creates the image (downloads Zephyr once).
#
#   zephyr/docker.sh build   # build every examples/*.c into build-zephyr/<name>
#   zephyr/docker.sh run     # run them all
#   zephyr/docker.sh test    # build + run every test/osal/*_test.c (cmocka) on native_sim
#   zephyr/docker.sh tidy    # clang-tidy over source/zephyr/*.c
#   zephyr/docker.sh shell   # interactive shell in the container
#
# EXAMPLE=<name> limits build/run to one file from examples/.
# TEST=<name> limits test to one file from test/osal/ (for example TEST=mutex_test).
set -euo pipefail

IMAGE=dmosal-zephyr:local
BOARD=native_sim/native/64
REPO="$(cd "$(dirname "$0")/.." && pwd)"

if [ -n "${EXAMPLE:-}" ]; then
	EXAMPLES=("$EXAMPLE")
else
	EXAMPLES=()
	for f in "$REPO"/examples/*.c; do EXAMPLES+=("$(basename "$f" .c)"); done
fi

if [ -n "${TEST:-}" ]; then
	TESTS=("$TEST")
else
	TESTS=()
	for f in "$REPO"/test/osal/*_test.c; do TESTS+=("$(basename "$f" .c)"); done
fi

DOCKER_RUN=(docker run --rm -u "$(id -u):$(id -g)" -e HOME=/tmp -e CMOCKA_DIR=/opt/cmocka -v "$REPO":/dmosal)

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
	docker build -t "$IMAGE" -f "$REPO/zephyr/Dockerfile" "$REPO/zephyr"
fi

case "${1:-}" in
build)
	for ex in "${EXAMPLES[@]}"; do
		"${DOCKER_RUN[@]}" "$IMAGE" west build -b "$BOARD" -d "build-zephyr/$ex" zephyr/app -p auto \
			-- -DEXAMPLE="$ex"
	done
	;;
run)
	for ex in "${EXAMPLES[@]}"; do
		echo "=== $ex ==="
		"${DOCKER_RUN[@]}" "$IMAGE" "build-zephyr/$ex/zephyr/zephyr.exe" -stop_at=3
	done
	;;
test)
	failed=()
	for t in "${TESTS[@]}"; do
		"${DOCKER_RUN[@]}" "$IMAGE" west build -b "$BOARD" -d "build-zephyr-test/$t" zephyr/app -p auto \
			-- -DTEST="$t" >/dev/null
		echo "=== $t ==="
		"${DOCKER_RUN[@]}" "$IMAGE" "build-zephyr-test/$t/zephyr/zephyr.exe" -stop_at=120 || failed+=("$t")
	done
	if [ ${#failed[@]} -ne 0 ]; then
		echo "FAILED: ${failed[*]}"
		exit 1
	fi
	;;
tidy)
	# clang does not know a few gcc-only flags that Zephyr passes, so they are dropped from the
	# compilation database.
	"${DOCKER_RUN[@]}" "$IMAGE" sh -c \
		"west build -b $BOARD -d build-zephyr-tidy zephyr/app -p auto -- -DEXAMPLE=queue_sample -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null &&
		mkdir -p build-zephyr-tidy/db &&
		sed -e 's/ -fno-freestanding//g; s/ -fno-reorder-functions//g' build-zephyr-tidy/compile_commands.json > build-zephyr-tidy/db/compile_commands.json &&
		clang-tidy -p build-zephyr-tidy/db --quiet --warnings-as-errors='*' source/zephyr/*.c"
	;;
shell)
	"${DOCKER_RUN[@]}" -it "$IMAGE" bash
	;;
*)
	sed -n "2,11p" "$0"
	exit 1
	;;
esac
