# example task
add_executable(task_sample task_sample.c)
target_link_libraries(task_sample ${DMOSAL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_dependencies(examples task_sample)

# example queue
add_executable(queue_sample queue_sample.c)
target_link_libraries(queue_sample ${DMOSAL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_dependencies(examples queue_sample)
install(
  TARGETS task_sample queue_sample
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
)
