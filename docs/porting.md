# Porting guide

`dmosal`'s design isolates OS-specific code into a **backend** directory
under `source/`. The rest of the library, the resource manager, LIFO,
logging, error strings, tmcheck, and top-level init, is portable C and
unchanged across backends.

The reference implementation lives at `source/linux/` and targets Linux /
glibc. `source/zephyr/` and `source/freertos/` are two RTOS backends
(see `zephyr/README.md` and `freertos/README.md`). This document walks
through what a new backend must provide.

## Files a backend must implement

All six live in `source/<backend>/`:

| File | Public API implemented |
|---|---|
| `osal_mutex.c` | `osal_mutex_init`, `osal_mutex_deinit`, `osal_mutex_create`, `osal_mutex_delete`, `osal_mutex_lock`, `osal_mutex_unlock`, `osal_mutex_use`, `osal_mutex_avail`. |
| `osal_sem.c` | `osal_sem_init`, `osal_sem_deinit`, `osal_sem_create`, `osal_sem_delete`, `osal_sem_post`, `osal_sem_wait`, `osal_sem_waittime`, `osal_sem_use`, `osal_sem_avail`. |
| `osal_task.c` | `osal_task_init`, `osal_task_deinit`, `osal_task_create`, `osal_task_delete`, `osal_task_use`, `osal_task_avail`. |
| `osal_timer.c` | `osal_timer_init`, `osal_timer_deinit`, `osal_timer_create`, `osal_timer_delete`, `osal_timer_start`, `osal_timer_stop`, `osal_timer_use`, `osal_timer_avail`. |
| `osal_queue.c` | `osal_queue_init`, `osal_queue_deinit`, `osal_queue_create`, `osal_queue_delete`, `osal_queue_send`, `osal_queue_recv`, `osal_queue_use`, `osal_queue_avail`. |
| `osal_time.c` | `osal_sleep`, `osal_usleep`, `osal_clock_time`. |

The public headers (`include/dmosal/osal_*.h`) are fixed, they define the
API the backend must satisfy.

## Contracts to preserve

For each function, honour the behavior documented in
[api-spec.md](api-spec.md), [thread-safety.md](thread-safety.md), and
[lifetime.md](lifetime.md). Concretely:

- **Pool ops must be thread-safe**: `_create` and `_delete` need to
  serialize their pool touches. On Linux we do this by threading the
  shared `osal_mutex_t *mutex` through each `_man_t`'s `osal_rm_t`. Reuse
  that pattern via `OSAL_RM_USEROBJMAN_DECLARE` / `_INIT` if the target
  OS has a mutex primitive.
- **Callbacks run on library-managed threads**, not the caller's:
  - Task handlers run to completion on a spawned thread; the exit
    (returning from the handler) must not free the pool slot, that only
    happens in `osal_task_delete`.
  - Timer callbacks may run on any backend-managed context. On systems
    without a helper-thread mechanism, provide one (e.g. a dedicated
    timer dispatch task).
- **`osal_timer_delete` must reject in-callback invocation.** Use the
  `in_callback` bool pattern from the Linux backend: set true around the
  callback invocation, log an error and return from delete if it's true.

## Steps

1. **Create the directory.**
   ```
   source/mybackend/
   ├── osal_mutex.c
   ├── osal_queue.c
   ├── osal_sem.c
   ├── osal_task.c
   ├── osal_time.c
   └── osal_timer.c
   ```

2. **Add a build file.** The root `CMakeLists.txt` builds the Linux
   backend only. Other backends have their own build entry point that
   compiles `source/*.c` (portable layer) plus `source/<backend>/*.c`
   and generates `osal_config.h` / `osal_version.h`: see
   `zephyr/CMakeLists.txt` (Zephyr module) and `freertos/CMakeLists.txt`
   for templates. Only one backend is compiled in at a time.

3. **Implement `osal_mutex.c` first.** Everything else depends on it:
   the shared mutex from this pool is passed to every other subsystem's
   init. See `source/linux/osal_mutex.c` for the reference:
   - `s_mutex_man` static holding the pool.
   - A bare backend-level mutex (`s_mutex_man.resrc_mutex`) is used
     inside `osal_mutex_create` because it can't recursively use its own
     pool.
   - `osal_mutex_create` returns NULL if `s_mutex_man.init == false`
     (guards against tests that call create-before-init).

4. **Implement `osal_sem.c`, `osal_task.c`, `osal_timer.c`, `osal_queue.c`,
   `osal_time.c`.** Each mirrors the same `_man_t` + `osal_rm_t` structure.
   Copy the Linux file and swap the OS calls.

5. **Wire in the build.** The build file from step 2 is the only place
   where the backend directory is referenced. Neither the public headers
   nor the portable `source/*.c` files hard-code a backend.

6. **Run the existing test suite.** The cmocka tests in `test/osal/`
   exercise only the public API and should pass unchanged. Tests specific
   to one backend live in `test/<backend>/` (for example `test/linux/`).
   See `zephyr/app` and `freertos/app` for how to run them on an RTOS. If they fail, the failure
   is almost always a lifetime / thread-safety contract violation.

## Non-portable knobs you may need

Some behaviors are inherently backend-specific and may need adjustment:

- **Log timestamp format**: `osal_log.c` uses `osal_clock_time` to build
  timestamps. If your backend's monotonic clock has coarser resolution,
  the microsecond field will just always show 0, no code change needed.
- **`osal_task_cfg_t` hints**: `stack_addr`, `stack_size`, `priority`,
  `name` are optional (0 / NULL means the default from `osal_config.h`:
  `OSAL_TASK_STACK_SIZE_DEFAULT`, `OSAL_TASK_PRIORITY_DEFAULT`). Honour them if your target
  OS supports them. Zephyr and FreeRTOS do, Linux ignores them.
- **Timer callback thread**: the Linux backend spawns one helper thread
  per expiration (`SIGEV_THREAD`). On memory-constrained targets, use a
  single dedicated timer-dispatch task that walks a pending list.

## Validation checklist

After the backend compiles and links, before you declare it done:

- [ ] Every test in `test/osal/` passes.
- [ ] `make check-valgrind` (or the target's equivalent): no leaks.
- [ ] TSan or platform equivalent: no data races.
- [ ] Manual: create a task, have it lock the shared mutex, then
      `osal_deinit()` after cleanup. Should not crash or hang.
- [ ] Manual: create a timer, `osal_timer_delete` from inside its own
      callback. Should log an error and leave the timer intact.
- [ ] Manual: create a queue with `qsize=1`, send, send again: second
      send returns `OSAL_E_QFULL`.
- [ ] Manual: `osal_sem_waittime` with 1 ms timeout on an unposted
      semaphore returns `OSAL_E_TIMEOUT` in roughly 1 ms.

## What NOT to do in a backend

- Don't expose backend-specific types (e.g. `pthread_t`, `mqd_t`) in
  public headers. Wrap them in the opaque `struct osal_task`, etc.
- Don't rely on the caller including OS-specific headers before including
  `osal.h`.
- Don't allocate from the heap on the hot path. The static-allocation
  ethos is core to the library's contract.
- Don't add new public functions in the backend directory. If a
  feature genuinely needs a new API, add it to the portable header and
  implement it in every backend.
