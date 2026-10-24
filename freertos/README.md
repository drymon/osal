# dmosal on FreeRTOS

The FreeRTOS backend ([source/freertos/](../source/freertos/)) implements the same API as the
Linux and Zephyr backends using the kernel's **static allocation** API when it is enabled (no heap needed), or the
dynamic API otherwise. It is tested on the FreeRTOS
[POSIX/Linux simulator port](https://www.freertos.org/FreeRTOS-simulator-for-Linux.html), which
compiles FreeRTOS and the application into a normal Linux executable, so CI needs no hardware
or emulator.

## Run

Everything runs inside Docker; only this repository is mounted. The first run builds the image
(FreeRTOS-Kernel is pinned in `freertos/Dockerfile`).

```
just freertos-build     # build every examples/*.c and test/osal/*_test.c into build-freertos/
just freertos-run       # run the examples
just freertos-test      # run the cmocka tests (TEST=mutex_test for one)
just freertos-shell     # shell in the container
```

The examples and the cmocka tests under `test/osal/` are the same sources used for Linux and
Zephyr. Each keeps its own `main()`: the build renames it and [app/main.c](app/main.c) runs it
in a FreeRTOS task, then exits the process with its result.

## Layout

| Path | Purpose |
|---|---|
| `source/freertos/` | the backend (`osal_mutex.c`, `osal_sem.c`, `osal_task.c`, `osal_timer.c`, `osal_queue.c`, `osal_time.c`) |
| `freertos/CMakeLists.txt` | the `dmosal` library target for FreeRTOS builds |
| `freertos/app/` | simulator application: `FreeRTOSConfig.h`, `main.c`, CMake project |
| `freertos/Dockerfile`, `freertos/docker.sh` | build environment and runner |

## Use it in your own FreeRTOS project

```cmake
add_subdirectory(path/to/dmosal/freertos dmosal)   # needs a `freertos_kernel` target
target_link_libraries(my_app PRIVATE dmosal)
```

The backend uses only the FreeRTOS user-space API, so it works with the kernel configuration your
SoC vendor ships. It adapts to it (see the top of
[source/freertos/osal_freertos_priv.h](../source/freertos/osal_freertos_priv.h)):

- `configSUPPORT_STATIC_ALLOCATION 1`: static API, no heap. Your application (or the vendor)
  provides `vApplicationGetIdleTaskMemory()` and `vApplicationGetTimerTaskMemory()`.
- otherwise: dynamic API (`configSUPPORT_DYNAMIC_ALLOCATION 1` and a heap implementation).
  The pools stay static; only the kernel objects come from the heap.
- always: `configUSE_MUTEXES`, `configUSE_COUNTING_SEMAPHORES`, `configUSE_TIMERS`,
  `INCLUDE_vTaskDelete`, `INCLUDE_vTaskSuspend`, `INCLUDE_xTaskGetCurrentTaskHandle` (a missing
  option stops the build with an `#error`).
- the timer service task priority must be higher than every task that deletes timers

`FreeRTOSConfig.h` and the idle/timer memory hooks in `freertos/app/` are only the simulator
harness; use the vendor's instead. `CMAKE_ARGS=-DDYNAMIC_ALLOC=ON BUILD_DIR=build-freertos-dyn
freertos/docker.sh build` builds the harness with the dynamic API (not run in CI).

All settings come from [osal_config.cmake](../osal_config.cmake) (`-DOSAL_CONFIG_TASK_NUM_MAX=8`,
...), the same file as for every backend. This backend uses `OSAL_CONFIG_TASK_STACK_SIZE_DEFAULT` (bytes),
`OSAL_CONFIG_TASK_PRIORITY_DEFAULT` (a FreeRTOS priority, higher is more urgent), `OSAL_CONFIG_QUEUE_MSG_MAX_SIZE`
and `OSAL_CONFIG_QUEUE_MSG_NUM_MAX`.

## Mapping and behaviour differences

| dmosal | FreeRTOS | Note |
|---|---|---|
| mutex | `xSemaphoreCreateMutexStatic` | non-recursive; unlock by a non-owner returns `OSAL_E_OSCALL` |
| sem | `xSemaphoreCreateCountingStatic` | |
| task | `xTaskCreateStatic` | one static stack per pool slot |
| timer | `xTimerCreateStatic` | `expire()` runs in the timer service task |
| queue | `xQueueCreateStatic` | in-process, shared by name like the Zephyr backend |
| time | tick count | resolution is one tick (`configTICK_RATE_HZ`, 1 ms in the simulator) |

- **Time resolution:** sleeps and timeouts are rounded up to whole ticks. `osal_clock_time()`
  has tick resolution, so tmcheck differences are multiples of one tick.
- **Task end:** when a task handler returns, the task is parked (a FreeRTOS task must not
  return). `osal_task_delete()` removes it; deleting the calling task itself is ignored, as the
  kernel only releases its TCB later.
- **Task priority and stack:** `priority`, `stack_size` and `stack_addr` of `osal_task_cfg_t` are
  honoured. 0 selects `OSAL_CONFIG_TASK_PRIORITY_DEFAULT` / `OSAL_CONFIG_TASK_STACK_SIZE_DEFAULT`. Without
  `stack_addr` the stack is embedded in the task object, so `stack_size` is limited to
  `OSAL_CONFIG_TASK_STACK_SIZE_DEFAULT`. With `-DDMOSAL_TASK_STACK_EMBEDDED=OFF` there is no
  embedded stack and `osal_task_create()` fails unless `stack_addr` is given. With dynamic
  allocation `stack_addr` is not used.
- **Timers:** deleting a timer from its own callback is ignored.
