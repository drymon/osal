cmake_minimum_required(VERSION 3.10.2)
project(dmosal C)

set (DMOSAL_MAJOR_VERSION 1)
set (DMOSAL_MINOR_VERSION 0)
set (DMOSAL_PATCH_VERSION 0)
set (DMOSAL_VERSION ${DMOSAL_MAJOR_VERSION}.${DMOSAL_MINOR_VERSION}.${DMOSAL_PATCH_VERSION})

find_package(Threads REQUIRED)

file(GLOB osal_SRC
  implementation/*.c
  implementation/posix/*.c
  )

include_directories(
  osal
  )

add_library(dmosal SHARED ${osal_SRC})
set_target_properties(dmosal PROPERTIES VERSION ${DMOSAL_VERSION} SOVERSION ${DMOSAL_MAJOR_VERSION})


add_executable(osal_task "example/task_example.c")
target_link_libraries(osal_task dmosal ${CMAKE_THREAD_LIBS_INIT})

install(
  TARGETS dmosal
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT bin
  )

add_custom_target(uninstall
  COMMAND xargs rm < ${CMAKE_BINARY_DIR}/install_manifest.txt
  )

enable_testing()
set(CMAKE_CTEST_COMMAND stdbuf -o0 ctest -V)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(check dmosal)
add_subdirectory(test EXCLUDE_FROM_ALL)
