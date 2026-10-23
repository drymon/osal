cmake_minimum_required(VERSION 3.20.0)

# Pick up the dmosal Zephyr module from this repository without a west manifest.
list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_LIST_DIR}/../..")

# Build one file from examples/ ...   west build ... -- -DEXAMPLE=task_sample
# ... or one cmocka test from test/osal/:   west build ... -- -DTEST=mutex_test
set(EXAMPLE queue_sample CACHE STRING "Example from examples/ to build")
set(TEST "" CACHE STRING "Test from test/osal/ to build instead of an example")
set(CMOCKA_DIR "$ENV{CMOCKA_DIR}" CACHE PATH "cmocka source tree (tests only)")

if(TEST)
  # cmocka needs signal.h, setenv() etc., which the minimal libc lacks: use the host libc.
  set(EXTRA_CONF_FILE ${CMAKE_CURRENT_LIST_DIR}/test.conf)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dmosal_zephyr_app)

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

if(TEST)
  target_sources(app PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_main.c
    ${REPO_ROOT}/test/osal/${TEST}.c
    ${CMOCKA_DIR}/src/cmocka.c)
  # The test keeps its own main(); test_main.c calls it and reports the result.
  set_source_files_properties(${REPO_ROOT}/test/osal/${TEST}.c
    PROPERTIES COMPILE_DEFINITIONS main=dmosal_test_main)
  # -std=c11 hides POSIX declarations (signal, setenv, strdup) that cmocka and the tests use.
  # HAVE_CONFIG_H makes cmocka.c read cmocka/config.h.
  target_compile_definitions(app PRIVATE _GNU_SOURCE HAVE_CONFIG_H)
  target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/cmocka ${CMOCKA_DIR}/include ${REPO_ROOT}/test)
else()
  target_sources(app PRIVATE ${REPO_ROOT}/examples/${EXAMPLE}.c)
endif()
