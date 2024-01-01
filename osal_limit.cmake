# The default configuration, it can be changed from the command line as:
# $ cmake -DOSAL_CONFIG_MUTEX_NUM_MAX=32 ../

set(OSAL_CONFIG_TASK_NAME_SIZE 64
    CACHE STRING "The length of the task name"
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
    CACHE STRING "Maximum number of semaphores to support"
)
