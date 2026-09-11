# Architecture

## Purpose

`dmosal` is a thin OS abstraction layer with a **static-allocation** ethos:
every resource (mutex, semaphore, task, timer, queue, tmcheck) comes from a
fixed-size pool sized at compile time. Applications acquire resources at
initialization, then run without further heap allocation. The layer is
designed for embedded / real-time contexts where predictable memory
behaviour matters more than dynamic flexibility.

## Layered view

```
   +-------------------------------+
   |         Application           |
   +-------------------------------+
   |          dmosal (public API)  |
   +---------------+---------------+
   |    portable   |    backend    |
   |    layer      |    (POSIX...) |
   +---------------+---------------+
   |         OS user-space API     |
   +-------------------------------+
   |         OS kernel space       |
   +-------------------------------+
   |             HW                |
   +-------------------------------+
```

- **Public API** — all `osal_*` symbols exposed via `include/dmosal/*.h`.
  Backend-agnostic; no POSIX / RTOS-specific names leak into headers.
- **Portable layer** — `source/*.c`. Data structures and logic shared
  across every backend (resource manager, LIFO free-list, error strings,
  logging, time-checkpoint utility, top-level init).
- **Backend** — `source/<backend>/*.c`. Only these files touch OS-specific
  APIs. The POSIX backend under `source/posix/` is the reference
  implementation.

## Subsystems

Each subsystem is a small module with its own header, source file, and
static resource pool.

| Subsystem | Public header | What it provides |
|---|---|---|
| Top-level | `osal.h` | `osal_init`, `osal_deinit`, config, version, resource print. |
| Mutex | `osal_mutex.h` | Recursive-free lock. Also used to serialize other subsystems' resource pools. |
| Semaphore | `osal_sem.h` | Counting semaphore with timed wait. |
| Task | `osal_task.h` | Thread/task with a handler function. |
| Timer | `osal_timer.h` | One-shot or periodic timer. Callback runs on a backend-managed context. |
| Queue | `osal_queue.h` | Named byte-oriented message queue. |
| Time | `osal_time.h` | Monotonic clock, sleep. |
| Log | `osal_log.h` | Per-module log levels, custom output sink, build-time level gating. |
| Assert | `osal_assert.h` | `OSAL_RUNTIME_ASSERT` (abort on failure) and `OSAL_STATIC_ASSERT` (compile-time). |
| Error | `osal_error.h` | `osal_error_t` enum + string decoder. |
| Resource manager | `osal_rm.h` | Generic fixed-pool allocator used internally by every other subsystem. |
| LIFO | `osal_lifo.h` | Intrusive stack; building block for the resource manager. |
| Tmcheck | `osal_tmcheck.h` | Named time checkpoints for micro-benchmarking. |

## The resource-manager pattern

Every "typed" subsystem (mutex, sem, task, timer, queue, tmcheck) is built
on the same skeleton:

```
+-------------------+           +---------------+
|  _man_t (static)  |           |   osal_rm_t   |
|                   |           |               |
|  osal_rm_t rm     |---------->|  head ptr     |----> free-list of
|  userobj[N]       |           |  n_resrces    |      osal_resrc_t
|  resrces[N]       |           |  mutex ptr    |
+-------------------+           +---------------+
```

- `userobj[N]` is the concrete struct (e.g. `struct osal_mutex[64]`).
- `resrces[N]` is the parallel array of `osal_resrc_t` wrappers.
- `osal_rm_t` holds a LIFO of free `osal_resrc_t` pointers.
- `osal_rm_alloc` pops from the LIFO; `osal_rm_free` pushes back.

Each subsystem declares its manager with `OSAL_RM_USEROBJMAN_DECLARE` and
initialises with `OSAL_RM_USEROBJMAN_INIT`, so the boilerplate is small.

The RM optionally takes an external mutex. In `osal_init`, every subsystem
gets the same **shared mutex** (`s_shared_mutex`), so pool operations across
subsystems serialize on a single lock. This is inexpensive when creates and
deletes are rare (init/shutdown) and avoids per-subsystem locking overhead.

## Callback threading model

Two subsystems invoke user code from a thread other than the caller's:

- **Task** — `osal_task_create(cfg)` spawns a new thread that runs
  `cfg->task_handler`. The handler runs to completion on that thread.
- **Timer** — the timer callback fires on a backend-managed context, not
  the caller's thread. On the POSIX backend that means a fresh helper
  thread per expiration (via `SIGEV_THREAD`). User code must not assume
  any particular thread identity inside the callback.

Everything else (mutex lock/unlock, semaphore post/wait, queue send/recv,
log print) runs on the caller's thread.

## Backend model

Adding a new backend means providing implementations for these files under
`source/<backend>/`:

- `osal_mutex.c`
- `osal_sem.c`
- `osal_task.c`
- `osal_timer.c`
- `osal_queue.c`
- `osal_time.c`

The portable files under `source/` (`osal.c`, `osal_error.c`, `osal_log.c`,
`osal_lifo.c`, `osal_rm.c`, `osal_tmcheck.c`) work unchanged. See
[porting.md](porting.md) for a step-by-step guide.
