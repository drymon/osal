cmake_minimum_required(VERSION 3.15)
project(dmosal_freertos_app C)

# Builds every example in examples/ and every cmocka test in test/osal/ as a host executable
# running on the FreeRTOS POSIX simulator, plus dmosal itself (../CMakeLists.txt).
#
#   FREERTOS_KERNEL_PATH   FreeRTOS-Kernel source tree (env or -D)
#
# Tests need libcmocka (apt install libcmocka-dev).

set(FREERTOS_KERNEL_PATH "$ENV{FREERTOS_KERNEL_PATH}" CACHE PATH "FreeRTOS-Kernel source tree")
if(NOT FREERTOS_KERNEL_PATH)
  message(FATAL_ERROR "Set FREERTOS_KERNEL_PATH to a FreeRTOS-Kernel checkout")
endif()

get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

option(DYNAMIC_ALLOC "Use the kernel's dynamic allocation API (heap_4) instead of static allocation" OFF)

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR})
if(DYNAMIC_ALLOC)
  target_compile_definitions(freertos_config INTERFACE DMOSAL_APP_DYNAMIC_ALLOC)
  set(FREERTOS_HEAP 4 CACHE STRING "")
endif()

set(FREERTOS_PORT GCC_POSIX CACHE STRING "")
add_subdirectory(${FREERTOS_KERNEL_PATH} freertos_kernel)
add_subdirectory(${REPO_ROOT}/freertos dmosal)

add_library(app_main OBJECT main.c)
target_link_libraries(app_main PRIVATE freertos_kernel)

# Each program keeps its own main(): it is renamed and run from a FreeRTOS task by main.c.
function(add_dmosal_program name source)
  add_executable(${name} ${source} $<TARGET_OBJECTS:app_main>)
  set_source_files_properties(${source} PROPERTIES COMPILE_DEFINITIONS main=dmosal_app_main)
  target_link_libraries(${name} PRIVATE dmosal)
endfunction()

enable_testing()
add_custom_target(examples)
add_custom_target(tests)

file(GLOB EXAMPLE_SOURCES ${REPO_ROOT}/examples/*.c)
foreach(src ${EXAMPLE_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_dmosal_program(${name} ${src})
  add_dependencies(examples ${name})
endforeach()

find_package(cmocka)
if(NOT cmocka_FOUND AND NOT CMOCKA_LIBRARIES)
  find_library(CMOCKA_LIBRARIES cmocka REQUIRED)
endif()

file(GLOB TEST_SOURCES ${REPO_ROOT}/test/osal/*_test.c)
foreach(src ${TEST_SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_dmosal_program(${name} ${src})
  target_include_directories(${name} PRIVATE ${REPO_ROOT}/test)
  target_link_libraries(${name} PRIVATE ${CMOCKA_LIBRARIES})
  add_dependencies(tests ${name})
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES TIMEOUT 120)
endforeach()
