# dmosal on FreeRTOS (static allocation only).
#
# Usage from an application that already provides the FreeRTOS kernel target:
#
#   add_subdirectory(<dmosal>/freertos dmosal)
#   target_link_libraries(my_app PRIVATE dmosal)
#
# The kernel target must be called `freertos_kernel` and be configured as described at the
# top of source/freertos/osal_freertos_priv.h. Pool sizes come from osal_config.cmake and can
# be overridden on the command line (-DOSAL_CONFIG_TASK_NUM_MAX=8 ...).

get_filename_component(DMOSAL_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

include("${DMOSAL_ROOT}/osal_config.cmake")

set(DMOSAL_MAJOR_VERSION 1)
set(DMOSAL_MINOR_VERSION 0)
set(DMOSAL_PATCH_VERSION 0)
execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${DMOSAL_ROOT}"
  OUTPUT_VARIABLE DMOSAL_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
if(NOT DMOSAL_GIT_COMMIT)
  set(DMOSAL_GIT_COMMIT "freertos")
endif()
string(TIMESTAMP DMOSAL_BUILD_TIME "%Y:%m:%d-%H:%M")

set(DMOSAL_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/dmosal")
file(MAKE_DIRECTORY "${DMOSAL_GEN_DIR}")
configure_file("${DMOSAL_ROOT}/include/dmosal/osal_config.h.in"
  "${DMOSAL_GEN_DIR}/osal_config.h" @ONLY)
configure_file("${DMOSAL_ROOT}/include/dmosal/osal_version.h.in"
  "${DMOSAL_GEN_DIR}/osal_version.h" @ONLY)

add_library(dmosal STATIC
  "${DMOSAL_ROOT}/source/osal.c"
  "${DMOSAL_ROOT}/source/osal_error.c"
  "${DMOSAL_ROOT}/source/osal_lifo.c"
  "${DMOSAL_ROOT}/source/osal_log.c"
  "${DMOSAL_ROOT}/source/osal_rm.c"
  "${DMOSAL_ROOT}/source/osal_tmcheck.c"
  "${DMOSAL_ROOT}/source/freertos/osal_mutex.c"
  "${DMOSAL_ROOT}/source/freertos/osal_queue.c"
  "${DMOSAL_ROOT}/source/freertos/osal_sem.c"
  "${DMOSAL_ROOT}/source/freertos/osal_task.c"
  "${DMOSAL_ROOT}/source/freertos/osal_time.c"
  "${DMOSAL_ROOT}/source/freertos/osal_timer.c"
)

target_include_directories(dmosal PUBLIC
  "${DMOSAL_ROOT}/include"
  "${DMOSAL_ROOT}/include/dmosal"
  "${CMAKE_CURRENT_BINARY_DIR}/gen"
  "${DMOSAL_GEN_DIR}"
)
target_include_directories(dmosal PRIVATE "${DMOSAL_ROOT}/source/freertos")

target_compile_options(dmosal PRIVATE
  -Wshadow -Wconversion -Wpedantic -Wcast-align -Wpointer-arith
  -Wstrict-prototypes -Wmissing-prototypes
)

target_link_libraries(dmosal PUBLIC freertos_kernel)
