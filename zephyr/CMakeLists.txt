if(CONFIG_DMOSAL)

get_filename_component(DMOSAL_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

set(DMOSAL_MAJOR_VERSION 1)
set(DMOSAL_MINOR_VERSION 0)
set(DMOSAL_PATCH_VERSION 0)
execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY "${DMOSAL_ROOT}"
  OUTPUT_VARIABLE DMOSAL_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
if(NOT DMOSAL_GIT_COMMIT)
  set(DMOSAL_GIT_COMMIT "zephyr")
endif()
string(TIMESTAMP DMOSAL_BUILD_TIME "%Y:%m:%d-%H:%M")

# Feed the existing osal_config.h.in template from Kconfig so the
# portable code sees the same OSAL_*_NUM_MAX macros as on other backends.
foreach(_opt
    TASK_NAME_SIZE TASK_NUM_MAX MUTEX_NUM_MAX SEM_NUM_MAX TIMER_NUM_MAX
    QUEUE_NAME_SIZE QUEUE_NUM_MAX LOG_MODULE_NUM_MAX LOG_MODULE_NAME_SIZE
    LOG_STRING_SIZE TMCHECK_NUM_MAX
    TASK_STACK_SIZE_DEFAULT TASK_PRIORITY_DEFAULT QUEUE_MSG_MAX_SIZE QUEUE_MSG_NUM_MAX)
  set(OSAL_CONFIG_${_opt} ${CONFIG_DMOSAL_${_opt}})
endforeach()
if(CONFIG_DMOSAL_TASK_STACK_EMBEDDED)
  set(OSAL_CONFIG_TASK_STACK_EMBEDDED 1)
else()
  set(OSAL_CONFIG_TASK_STACK_EMBEDDED 0)
endif()

set(DMOSAL_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/dmosal")
file(MAKE_DIRECTORY "${DMOSAL_GEN_DIR}")
configure_file("${DMOSAL_ROOT}/include/dmosal/osal_config.h.in"
  "${DMOSAL_GEN_DIR}/osal_config.h" @ONLY)
configure_file("${DMOSAL_ROOT}/include/dmosal/osal_version.h.in"
  "${DMOSAL_GEN_DIR}/osal_version.h" @ONLY)

zephyr_library_named(dmosal)

zephyr_include_directories(
  "${DMOSAL_ROOT}/include"
  "${DMOSAL_ROOT}/include/dmosal"
  "${CMAKE_CURRENT_BINARY_DIR}/gen"
  "${DMOSAL_GEN_DIR}"
)

zephyr_library_sources(
  "${DMOSAL_ROOT}/source/osal.c"
  "${DMOSAL_ROOT}/source/osal_error.c"
  "${DMOSAL_ROOT}/source/osal_lifo.c"
  "${DMOSAL_ROOT}/source/osal_log.c"
  "${DMOSAL_ROOT}/source/osal_rm.c"
  "${DMOSAL_ROOT}/source/osal_tmcheck.c"
  "${DMOSAL_ROOT}/source/zephyr/osal_mutex.c"
  "${DMOSAL_ROOT}/source/zephyr/osal_queue.c"
  "${DMOSAL_ROOT}/source/zephyr/osal_sem.c"
  "${DMOSAL_ROOT}/source/zephyr/osal_task.c"
  "${DMOSAL_ROOT}/source/zephyr/osal_time.c"
  "${DMOSAL_ROOT}/source/zephyr/osal_timer.c"
)

endif()
