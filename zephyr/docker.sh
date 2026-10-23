#!/usr/bin/env bash
# Build and run dmosal's Zephyr examples on native_sim, entirely inside Docker.
# The image (zephyr/Dockerfile) contains Zephyr; only this repo is mounted.
# The first build creates the image (downloads Zephyr once).
#
#   zephyr/docker.sh build   # build every examples/*.c into build-zephyr/<name>
#   zephyr/docker.sh run     # run them all
#   zephyr/docker.sh shell   # interactive shell in the container
#
# EXAMPLE=<name> limits build/run to one file from examples/.
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

DOCKER_RUN=(docker run --rm -u "$(id -u):$(id -g)" -e HOME=/tmp -v "$REPO":/dmosal)

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
shell)
	"${DOCKER_RUN[@]}" -it "$IMAGE" bash
	;;
*)
	sed -n "2,10p" "$0"
	exit 1
	;;
esac
