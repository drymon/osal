find_package(cmocka REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TASK_TEST task_test)
add_executable(${TASK_TEST} osal/task_test.c)
target_link_libraries(${TASK_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${TASK_TEST})
add_test(${TASK_TEST} ${TASK_TEST})

set(RM_TEST rm_test)
add_executable(${RM_TEST} osal/rm_test.c)
target_link_libraries(${RM_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${RM_TEST})
add_test(${RM_TEST} ${RM_TEST})

set(MUTEX_TEST mutex_test)
add_executable(${MUTEX_TEST} osal/mutex_test.c)
target_link_libraries(${MUTEX_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${MUTEX_TEST})
add_test(${MUTEX_TEST} ${MUTEX_TEST})

set(LIFO_TEST lifo_test)
add_executable(${LIFO_TEST} osal/lifo_test.c)
target_link_libraries(${LIFO_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${LIFO_TEST})
add_test(${LIFO_TEST} ${LIFO_TEST})

set(ERROR_TEST error_test)
add_executable(${ERROR_TEST} osal/error_test.c)
target_link_libraries(${ERROR_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${ERROR_TEST})
add_test(${ERROR_TEST} ${ERROR_TEST})

set(SEM_TEST sem_test)
add_executable(${SEM_TEST} osal/sem_test.c)
target_link_libraries(${SEM_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${SEM_TEST})
add_test(${SEM_TEST} ${SEM_TEST})

set(QUEUE_TEST queue_test)
add_executable(${QUEUE_TEST} osal/queue_test.c)
target_link_libraries(${QUEUE_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${QUEUE_TEST})
add_test(${QUEUE_TEST} ${QUEUE_TEST})

set(TIMER_TEST timer_test)
add_executable(${TIMER_TEST} osal/timer_test.c)
target_link_libraries(${TIMER_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${TIMER_TEST})
# glibc's SIGEV_THREAD timer notification (used by osal_timer) relies on
# internal real-time signals that conflict with ThreadSanitizer's own
# thread-suspension signals, causing a SIGSEGV unrelated to any real data
# race. Still build the binary under TSan (to catch compile issues) but
# don't register it with ctest in that configuration.
if(NOT DMOSAL_ENABLE_TSAN)
  add_test(${TIMER_TEST} ${TIMER_TEST})
endif()

set(LOG_TEST log_test)
add_executable(${LOG_TEST} osal/log_test.c)
target_link_libraries(${LOG_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${LOG_TEST})
add_test(${LOG_TEST} ${LOG_TEST})

set(TMCHECK_TEST tmcheck_test)
add_executable(${TMCHECK_TEST} osal/tmcheck_test.c)
target_link_libraries(${TMCHECK_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${TMCHECK_TEST})
add_test(${TMCHECK_TEST} ${TMCHECK_TEST})

set(TIME_TEST time_test)
add_executable(${TIME_TEST} osal/time_test.c)
target_link_libraries(${TIME_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${TIME_TEST})
add_test(${TIME_TEST} ${TIME_TEST})

set(OSAL_TEST osal_test)
add_executable(${OSAL_TEST} osal/osal_test.c)
target_link_libraries(${OSAL_TEST} dmosal ${CMOCKA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(check ${OSAL_TEST})
add_test(${OSAL_TEST} ${OSAL_TEST})
