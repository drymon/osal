cmake_minimum_required(VERSION 3.10)
project(dmosal C)

# OSAL version
set(DMOSAL_MAJOR_VERSION 1)
set(DMOSAL_MINOR_VERSION 0)
set(DMOSAL_PATCH_VERSION 0)
set(DMOSAL_VERSION ${DMOSAL_MAJOR_VERSION}.${DMOSAL_MINOR_VERSION}.${DMOSAL_PATCH_VERSION})
set(DMOSAL_LIBRARIES dmosal)

##############################################################################
# options
##############################################################################
option(DMOSAL_BUILD_SHARED    "Build a shared library (OFF => static)"      ON)
option(DMOSAL_ENABLE_ASAN     "Enable Address + UndefinedBehavior sanitizers" OFF)
option(DMOSAL_ENABLE_TSAN     "Enable Thread sanitizer"                     OFF)
option(DMOSAL_ENABLE_COVERAGE "Enable gcov coverage instrumentation"        OFF)

if(DMOSAL_ENABLE_ASAN AND DMOSAL_ENABLE_TSAN)
  message(FATAL_ERROR "DMOSAL_ENABLE_ASAN and DMOSAL_ENABLE_TSAN are mutually exclusive")
endif()

# Sanitizer runtimes (libasan / libtsan) must be the first library loaded
# in the process. With a shared dmosal.so, transitive load order can put
# other DSOs first and the sanitizer aborts. Forcing a static build when
# a sanitizer is on links the runtime directly into each executable and
# avoids the runtime-ordering problem entirely.
if((DMOSAL_ENABLE_ASAN OR DMOSAL_ENABLE_TSAN) AND DMOSAL_BUILD_SHARED)
  message(STATUS "Sanitizer enabled: forcing DMOSAL_BUILD_SHARED=OFF for a self-contained runtime")
  set(DMOSAL_BUILD_SHARED OFF)
endif()

##############################################################################
# build lib
##############################################################################
# OSAL resource limits
include("${CMAKE_CURRENT_SOURCE_DIR}/osal_config.cmake")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror=implicit-function-declaration")

# Extra warnings for the library sources only (verified clean on gcc and
# clang). Not applied to tests/examples since cmocka/test code is outside
# our control for some of these (e.g. -Wpedantic on generated test macros).
set(DMOSAL_EXTRA_WARNING_FLAGS
  -Wshadow
  -Wconversion
  -Wpedantic
  -Wcast-align
  -Wpointer-arith
  -Wstrict-prototypes
  -Wmissing-prototypes
)

# Sanitizers / coverage — one flag string shared between compile and link.
set(_dm_extra "")
if(DMOSAL_ENABLE_ASAN)
  set(_dm_extra "${_dm_extra} -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -O1")
endif()
if(DMOSAL_ENABLE_TSAN)
  set(_dm_extra "${_dm_extra} -fsanitize=thread -fno-omit-frame-pointer -O1")
endif()
if(DMOSAL_ENABLE_COVERAGE)
  set(_dm_extra "${_dm_extra} --coverage -O0")
endif()
if(_dm_extra)
  set(CMAKE_C_FLAGS             "${CMAKE_C_FLAGS} ${_dm_extra}")
  set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} ${_dm_extra}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${_dm_extra}")
endif()

find_package(Threads REQUIRED)

# Explicit source list — new files require a cmake refresh (on purpose).
set(osal_SRC
  source/osal.c
  source/osal_error.c
  source/osal_lifo.c
  source/osal_log.c
  source/osal_rm.c
  source/osal_tmcheck.c
  source/posix/osal_mutex.c
  source/posix/osal_queue.c
  source/posix/osal_sem.c
  source/posix/osal_task.c
  source/posix/osal_time.c
  source/posix/osal_timer.c
)

# Generated headers live under the build tree — never in the source tree.
set(DMOSAL_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/dmosal")
file(MAKE_DIRECTORY "${DMOSAL_GEN_DIR}")

execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE DMOSAL_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
string(TIMESTAMP DMOSAL_BUILD_TIME "%Y:%m:%d-%H:%M")

configure_file("include/dmosal/osal_config.h.in"
  "${DMOSAL_GEN_DIR}/osal_config.h" @ONLY)
configure_file("include/dmosal/osal_version.h.in"
  "${DMOSAL_GEN_DIR}/osal_version.h" @ONLY)
configure_file(dmosal.pc.in ${PROJECT_BINARY_DIR}/dmosal.pc @ONLY)

include_directories(
  include
  include/dmosal
  "${CMAKE_CURRENT_BINARY_DIR}/gen"
  "${DMOSAL_GEN_DIR}"
)

if(DMOSAL_BUILD_SHARED)
  add_library(dmosal SHARED ${osal_SRC})
else()
  add_library(dmosal STATIC ${osal_SRC})
endif()
target_link_libraries(dmosal ${CMAKE_THREAD_LIBS_INIT} rt)
target_compile_options(dmosal PRIVATE ${DMOSAL_EXTRA_WARNING_FLAGS})
set_target_properties(dmosal PROPERTIES
  VERSION ${DMOSAL_VERSION} SOVERSION ${DMOSAL_MAJOR_VERSION})

##############################################################################
# install & uninstall
##############################################################################
set(INSTALL_LIB_DIR     lib              CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR     bin              CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include          CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR   lib/cmake/dmosal CACHE PATH "Installation directory for CMake file")

# Public headers: hand-written + generated.
install(DIRECTORY include/dmosal/ "${DMOSAL_GEN_DIR}/"
  DESTINATION "${INSTALL_INCLUDE_DIR}/dmosal"
  COMPONENT dev
  FILES_MATCHING PATTERN "*.h"
)

install(FILES ${PROJECT_BINARY_DIR}/dmosal.pc
  DESTINATION ${INSTALL_LIB_DIR}/pkgconfig/ COMPONENT dev)

install(TARGETS dmosal EXPORT dmosalTargets
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT bin
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT bin
)

foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(ABSOLUTE_${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()
file(RELATIVE_PATH REL_INCLUDE_DIR
  "${ABSOLUTE_INSTALL_CMAKE_DIR}" "${ABSOLUTE_INSTALL_INCLUDE_DIR}/..")
set(CONF_INCLUDE_DIRS "\${DMOSAL_CMAKE_DIR}/${REL_INCLUDE_DIR}")

configure_file(dmosalConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/dmosalConfig.cmake" @ONLY)
configure_file(dmosalConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/dmosalConfigVersion.cmake" @ONLY)

install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/dmosalConfig.cmake"
    "${PROJECT_BINARY_DIR}/dmosalConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
install(EXPORT dmosalTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

add_custom_target(uninstall
  COMMAND sh -c "test -f '${CMAKE_BINARY_DIR}/install_manifest.txt' \
                 && xargs -r rm -f < '${CMAKE_BINARY_DIR}/install_manifest.txt' \
                 || echo 'no install_manifest.txt (nothing to uninstall)'"
)

##############################################################################
# tests
##############################################################################
# Memcheck config must be set as CACHE variables BEFORE include(CTest) so
# it makes it into DartConfiguration.tcl, which ctest -T memcheck reads.
find_program(VALGRIND_BIN valgrind)
if(VALGRIND_BIN)
  set(MEMORYCHECK_COMMAND "${VALGRIND_BIN}"
      CACHE FILEPATH "Memory-check command")
  set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1"
      CACHE STRING "Memory-check command options")
  set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/test/valgrind.supp"
      CACHE FILEPATH "Memory-check suppressions file")
endif()

# include(CTest) generates DartConfiguration.tcl and calls enable_testing().
include(CTest)

add_custom_target(check COMMAND ctest -V)
add_dependencies(check dmosal)

# Tests require cmocka; wrap in BUILD_TESTING so docs-only / production
# consumers can configure without the cmocka dependency
# (pass -DBUILD_TESTING=OFF). BUILD_TESTING is set by include(CTest) above.
# Collect the test target names registered by add_subdirectory(test) so the
# extra check-* targets can depend on them and actually build the binaries.
# When BUILD_TESTING=OFF the test/ directory isn't added, so this stays empty.
set(_dm_tests "")
if(BUILD_TESTING)
  add_subdirectory(test EXCLUDE_FROM_ALL)
  get_property(_dm_tests DIRECTORY test PROPERTY TESTS)
endif()

if(VALGRIND_BIN)
  add_custom_target(check-valgrind
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}
      ctest --output-on-failure -T memcheck)
  add_dependencies(check-valgrind dmosal ${_dm_tests})
endif()

find_program(LCOV_BIN    lcov)
find_program(GENHTML_BIN genhtml)
if(DMOSAL_ENABLE_COVERAGE AND LCOV_BIN AND GENHTML_BIN)
  add_custom_target(check-coverage
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} ctest --output-on-failure
    COMMAND ${LCOV_BIN} --capture -d ${CMAKE_BINARY_DIR}
      -o coverage.info --rc geninfo_unexecuted_blocks=1
    COMMAND ${LCOV_BIN} --remove coverage.info '/usr/*' '*/test/*' '*/examples/*'
      -o coverage.filtered.info
    COMMAND ${GENHTML_BIN} coverage.filtered.info -o coverage
    COMMENT "Coverage HTML: ${CMAKE_BINARY_DIR}/coverage/index.html"
  )
  add_dependencies(check-coverage dmosal ${_dm_tests})
endif()

##############################################################################
# examples
##############################################################################
add_custom_target(examples)
add_subdirectory(examples EXCLUDE_FROM_ALL)

##############################################################################
# doc
##############################################################################
add_custom_target(doc)
find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen is not found!")
else()
  configure_file(doxygen.cfg.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doxygen-doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
  add_dependencies(doc doxygen-doc)
endif()
