cmake_minimum_required(VERSION 3.20.0)

# Pick up the dmosal Zephyr module from this repository without a west manifest.
list(APPEND ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_LIST_DIR}/../..")

# Which file in examples/ to build: west build ... -- -DEXAMPLE=task_sample
set(EXAMPLE queue_sample CACHE STRING "Example from examples/ to build")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dmosal_zephyr_app)

target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../examples/${EXAMPLE}.c)
