# Convenience command runner around the CMake build so common workflows
# don't require remembering `mkdir build && cd build && cmake .. && make ...`.
#
# Install: https://github.com/casey/just#installation
# List all recipes: `just` or `just --list`
#
# Common usage:
#   just build              # configure (if needed) + build the library
#   just check              # build + run the full test suite
#   just check-valgrind     # build + run tests under valgrind memcheck
#   just check-asan         # build + run tests with ASan/UBSan enabled
#   just check-tsan         # build + run tests with ThreadSanitizer enabled
#   just check-coverage     # build + run tests with gcov coverage, generate html
#   just doc                # generate Doxygen HTML docs
#   just format             # apply clang-format to the whole tree
#   just tidy               # run clang-tidy
#   just clean              # remove build directories

build_dir := "build"
build_type := "Release"
jobs := `nproc 2>/dev/null || echo 4`

# List available recipes
default:
    @just --list

# Configure the CMake build (idempotent)
configure *args:
    cmake -S . -B {{build_dir}} -DCMAKE_BUILD_TYPE={{build_type}} {{args}}

# Build the library (and tests, examples where applicable)
build: configure
    cmake --build {{build_dir}} -j{{jobs}}

# Build and run the full test suite
check: configure
    cmake --build {{build_dir}} --target check -j{{jobs}}

# Build and run tests under valgrind memcheck (uses test/valgrind.supp)
check-valgrind: configure
    cmake --build {{build_dir}} --target check-valgrind -j{{jobs}}

# Build and run tests with Address/UndefinedBehavior sanitizers
check-asan:
    just build_dir=build-asan configure -DDMOSAL_ENABLE_ASAN=ON
    just build_dir=build-asan check

# Build and run tests with ThreadSanitizer
check-tsan:
    just build_dir=build-tsan configure -DDMOSAL_ENABLE_TSAN=ON
    just build_dir=build-tsan check

# Build and run tests with gcov coverage, generate HTML report
check-coverage:
    just configure -DDMOSAL_ENABLE_COVERAGE=ON
    cmake --build {{build_dir}} --target check-coverage -j{{jobs}}

# Generate Doxygen HTML documentation
doc: configure
    cmake --build {{build_dir}} --target doc

# Apply clang-format to the whole tree
format:
    pre-commit run clang-format --all-files

# Run clang-tidy
tidy:
    pre-commit run clang-tidy --all-files

# Remove the default build directory
clean:
    rm -rf {{build_dir}}

# Remove all build directories (default + sanitizer variants)
distclean:
    rm -rf {{build_dir}} build-asan build-tsan
