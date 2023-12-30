cmake_minimum_required(VERSION 2.8.12)
project(osal_example C)

find_package(Threads REQUIRED)
find_package(dmosal REQUIRED)

include_directories(${DMOSAL_INCLUDE_DIRS})

add_executable(task_example task_example.c)
target_link_libraries(task_example ${DMOSAL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(
  TARGETS task_example
  RUNTIME DESTINATION bin COMPONENT bin
)

add_custom_target(uninstall
  COMMAND xargs rm < ${CMAKE_BINARY_DIR}/install_manifest.txt
)
