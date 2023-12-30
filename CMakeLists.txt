cmake_minimum_required(VERSION 3.10.2)
project(osal C)

find_package(Threads REQUIRED)

file(GLOB osal_SRC
  implementation/*.c
  implementation/posix/*.c
  )

include_directories(
  osal
  )

add_library(dmosal SHARED ${osal_SRC})

enable_testing()
set(CMAKE_CTEST_COMMAND stdbuf -o0 ctest -V)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(check dmosal)
add_subdirectory(test EXCLUDE_FROM_ALL)
