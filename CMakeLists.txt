cmake_minimum_required(VERSION 2.8.12)
project(dmosal C)

# OSAL version
set(DMOSAL_MAJOR_VERSION 1)
set(DMOSAL_MINOR_VERSION 0)
set(DMOSAL_PATCH_VERSION 0)
set(DMOSAL_VERSION ${DMOSAL_MAJOR_VERSION}.${DMOSAL_MINOR_VERSION}.${DMOSAL_PATCH_VERSION})
set(DMOSAL_LIBRARIES dmosal)

##############################################################################
# build lib
##############################################################################
# OSAL resource limit
include("${CMAKE_CURRENT_SOURCE_DIR}/osal_config.cmake")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror=implicit-function-declaration")

find_package(Threads REQUIRED)

file(GLOB osal_SRC
  source/*.c
  source/posix/*.c
)

include_directories(
  include
  include/dmosal
)

add_library(dmosal SHARED ${osal_SRC})
set_target_properties(dmosal PROPERTIES VERSION
  ${DMOSAL_VERSION} SOVERSION ${DMOSAL_MAJOR_VERSION})

##############################################################################
# install & uninstall
##############################################################################
# OSAL install dirs
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR lib/cmake/dmosal CACHE PATH "Installation directory for CMake file")

set(DMOSAL_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/dmosal")

file(GLOB_RECURSE dmosal_INCLUDE RELATIVE ${DMOSAL_INCLUDE_PATH} "include/dmosal/*.h")
list(SORT dmosal_INCLUDE)

execute_process(COMMAND
  git rev-parse --short HEAD
  OUTPUT_VARIABLE
  DMOSAL_GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(TIMESTAMP DMOSAL_BUILD_TIME "%Y:%m:%d-%H:%M")


foreach(file ${dmosal_INCLUDE})
  get_filename_component( dir ${file} DIRECTORY)
  install(FILES "${DMOSAL_INCLUDE_PATH}/${file}"
	DESTINATION "${INSTALL_INCLUDE_DIR}/dmosal/${dir}" COMPONENT dev)
endforeach()

configure_file("${DMOSAL_INCLUDE_PATH}/osal_config.h.in"
  "${DMOSAL_INCLUDE_PATH}/osal_config.h" @ONLY)
configure_file("${DMOSAL_INCLUDE_PATH}/osal_version.h.in"
  "${DMOSAL_INCLUDE_PATH}/osal_version.h" @ONLY)
configure_file(dmosal.pc.in ${PROJECT_BINARY_DIR}/dmosal.pc @ONLY)

install(FILES ${PROJECT_BINARY_DIR}/dmosal.pc
  DESTINATION ${INSTALL_LIB_DIR}/pkgconfig/ COMPONENT dev)

install(
  TARGETS dmosal
  EXPORT dmosalTargets
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

install(
  FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/dmosalConfig.cmake"
  "${PROJECT_BINARY_DIR}/dmosalConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}"
  COMPONENT dev
)

install(
  EXPORT dmosalTargets
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev
)

# uninstall
add_custom_target(uninstall
  COMMAND xargs rm < ${CMAKE_BINARY_DIR}/install_manifest.txt
)

##############################################################################
# build test
##############################################################################
enable_testing()
set(CMAKE_CTEST_COMMAND stdbuf -o0 ctest -V)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_dependencies(check dmosal)
add_subdirectory(test EXCLUDE_FROM_ALL)

##############################################################################
# build examples
##############################################################################
add_custom_target(examples)
add_subdirectory(examples EXCLUDE_FROM_ALL)

##############################################################################
# build doc
##############################################################################
add_custom_target(doc)

find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
  message(WARNING "Doxygen is not found!")
else()
  configure_file(doc/doxygen.cfg.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doxygen-doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
  add_dependencies(doc doxygen-doc)
endif()
