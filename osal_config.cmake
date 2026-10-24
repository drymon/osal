# The default configuration, it can be changed from the command line as:
# $ cmake -DOSAL_CONFIG_MUTEX_NUM_MAX=32 ../

set(OSAL_CONFIG_TASK_NAME_SIZE 64
    CACHE STRING "The size of the task name"
)

set(OSAL_CONFIG_TASK_STACK_SIZE_DEFAULT 16384
    CACHE STRING "Stack size of each task in bytes (RTOS backends; no effect on Linux)"
)

set(DMOSAL_TASK_STACK_EMBEDDED ON
    CACHE BOOL "Each task object embeds a stack array. OFF: the caller must pass cfg->stack_addr (RTOS backends; no effect on Linux)"
)
if(DMOSAL_TASK_STACK_EMBEDDED)
  set(OSAL_CONFIG_TASK_STACK_EMBEDDED 1)
else()
  set(OSAL_CONFIG_TASK_STACK_EMBEDDED 0)
endif()

set(OSAL_CONFIG_TASK_PRIORITY_DEFAULT 1
    CACHE STRING "Native priority of tasks (RTOS backends; no effect on Linux)"
)

set(OSAL_CONFIG_TASK_NUM_MAX 64
    CACHE STRING "Maximum of number tasks to support"
)

set(OSAL_CONFIG_MUTEX_NUM_MAX 64
    CACHE STRING "Maximum number of mutexes to support"
)

set(OSAL_CONFIG_SEM_NUM_MAX 64
    CACHE STRING "Maximum number of semaphores to support"
)

set(OSAL_CONFIG_TIMER_NUM_MAX 64
    CACHE STRING "Maximum number of timers to support"
)

set(OSAL_CONFIG_QUEUE_NAME_SIZE 64
    CACHE STRING "The size of the queue name"
)

set(OSAL_CONFIG_QUEUE_NUM_MAX 64
    CACHE STRING "Maximum number of queues to support"
)

set(OSAL_CONFIG_QUEUE_MSG_MAX_SIZE 256
    CACHE STRING "Maximum message length of a queue in bytes (RTOS backends; no effect on Linux)"
)

set(OSAL_CONFIG_QUEUE_MSG_NUM_MAX 8
    CACHE STRING "Maximum number of messages per queue (RTOS backends; no effect on Linux)"
)

set(OSAL_CONFIG_LOG_MODULE_NUM_MAX 32
    CACHE STRING "Maximum number of log module to support"
)

set(OSAL_CONFIG_LOG_MODULE_NAME_SIZE 64
    CACHE STRING "The size of the log module name"
)

set(OSAL_CONFIG_LOG_STRING_SIZE 256
    CACHE STRING "The size of the log string buffer"
)

set(OSAL_CONFIG_TMCHECK_NUM_MAX 64
    CACHE STRING "Maximum number of the time check point to support"
)
