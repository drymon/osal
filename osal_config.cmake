# The default configuration, it can be changed from the command line as:
# $ cmake -DOSAL_CONFIG_MUTEX_NUM_MAX=32 ../

set(OSAL_CONFIG_TASK_NAME_SIZE 64
    CACHE STRING "The size of the task name"
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

set(OSAL_CONFIG_LOG_MODULE_NUM_MAX 32
    CACHE STRING "Maximum number of log module to support"
)

set(OSAL_CONFIG_LOG_MODULE_NAME_SIZE 64
    CACHE STRING "The size of the log module name"
)

set(OSAL_CONFIG_LOG_STRING_SIZE 1024
    CACHE STRING "The size of the log string buffer"
)
