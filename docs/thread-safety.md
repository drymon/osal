# Thread-safety contract

Every public `osal_*` function falls into one of four categories:

- **[any]** — Safe to call concurrently from multiple threads on the same
  or different objects, and against other **[any]** calls in this table.
- **[owner]** — Multiple threads may call this, but only one thread may
  interact with a given object at a time. The library does not serialize
  concurrent calls on the same object; the caller must.
- **[init]** — Called once at program start / shutdown from the init
  thread. Not safe to race against any other `osal_*` call.
- **[callback]** — Invoked by the library on a backend-managed thread, not
  the caller's. Inside the callback, other `osal_*` calls follow their
  own row's rule, subject to the callback-context restrictions in
  [lifetime.md](lifetime.md).

## Top-level

| Function | Class |
|---|---|
| `osal_init` | **[init]** |
| `osal_deinit` | **[init]** |
| `osal_version` | **[any]** |
| `osal_print_resource` | **[any]** |

## Error

| Function | Class |
|---|---|
| `osal_errstr` | **[any]** — returns a static string, never NULL |

## Time

| Function | Class |
|---|---|
| `osal_sleep`, `osal_usleep` | **[any]** — blocks the calling thread only |
| `osal_clock_time` | **[any]** |

## Mutex

| Function | Class |
|---|---|
| `osal_mutex_init`, `osal_mutex_deinit` | **[init]** |
| `osal_mutex_create` | **[any]** — pool ops are internally serialized |
| `osal_mutex_delete` | **[owner]** — no thread may hold or lock the mutex being deleted |
| `osal_mutex_lock`, `osal_mutex_unlock` | **[any]** — the mutex itself provides the serialization |
| `osal_mutex_use`, `osal_mutex_avail` | **[any]** |

## Semaphore

| Function | Class |
|---|---|
| `osal_sem_init`, `osal_sem_deinit` | **[init]** |
| `osal_sem_create` | **[any]** |
| `osal_sem_delete` | **[owner]** — no thread may be waiting on or posting to it |
| `osal_sem_post`, `osal_sem_wait`, `osal_sem_waittime` | **[any]** — the semaphore itself is atomic |
| `osal_sem_use`, `osal_sem_avail` | **[any]** |

## Task

| Function | Class |
|---|---|
| `osal_task_init`, `osal_task_deinit` | **[init]** |
| `osal_task_create` | **[any]** — pool ops serialized |
| `osal_task_delete` | **[owner]** — must not race concurrent delete on the same task |
| `osal_task_use`, `osal_task_avail` | **[any]** |
| `task_handler` (user callback) | **[callback]** — runs on its own thread |

## Timer

| Function | Class |
|---|---|
| `osal_timer_init`, `osal_timer_deinit` | **[init]** |
| `osal_timer_create` | **[any]** |
| `osal_timer_delete` | **[owner]** — asserts if the timer's callback is currently running (from any thread). Stop the timer first, wait for any in-flight callback to return, then delete. |
| `osal_timer_start`, `osal_timer_stop` | **[owner]** — per-timer serialization required |
| `osal_timer_use`, `osal_timer_avail` | **[any]** |
| `expire` (user callback) | **[callback]** — runs on a backend-managed thread |

## Queue

| Function | Class |
|---|---|
| `osal_queue_init`, `osal_queue_deinit` | **[init]** |
| `osal_queue_create` | **[any]** |
| `osal_queue_delete` | **[owner]** — no thread may be sending or receiving |
| `osal_queue_send`, `osal_queue_recv` | **[any]** — the queue itself provides serialization between multiple senders and receivers |
| `osal_queue_use`, `osal_queue_avail` | **[any]** |

## Log

| Function | Class |
|---|---|
| `osal_log_init`, `osal_log_deinit` | **[init]** |
| `osal_log_module_init` | **[init]** — call at startup for each module |
| `osal_log_module_change` | **[any]** — cheap level change at runtime |
| `osal_log_print` / `OSALOG_*` macros | **[any]** — the caller-provided output function must itself be thread-safe |

**Note:** the internal formatted string is built on a per-call stack buffer,
so different `osal_log_print` calls don't interfere with each other. Message
interleaving in the output stream (e.g. `printf` to stdout) depends on the
output-function contract — the default `printf` sink is line-buffered and
generally safe on most systems.

## Resource manager (public but rarely used by application code)

| Function | Class |
|---|---|
| `osal_rm_init` | **[init]** |
| `osal_rm_deinit` | **[owner]** — see [lifetime.md](lifetime.md) for ordering vs. the RM's mutex |
| `osal_rm_alloc`, `osal_rm_free` | **[any]** if the RM was init'd with a mutex; **[owner]** otherwise |
| `osal_rm_use`, `osal_rm_avail` | same as `alloc`/`free` |

## LIFO

| Function | Class |
|---|---|
| All `osal_lifo_*` | **[owner]** — the LIFO API is intentionally lock-free; caller synchronizes |

## Tmcheck

| Function | Class |
|---|---|
| `osal_tmcheck_init`, `osal_tmcheck_deinit` | **[init]** |
| `osal_tmcheck_create`, `osal_tmcheck_delete` | **[any]** — internally locked |
| `osal_tmcheck_capture_ts`, `osal_tmcheck_reset`, `osal_tmcheck_reset_all`, all `osal_tmcheck_*_print*`, `osal_tmcheck_get_diff`, `osal_tmcheck_name_get_diff`, `osal_tmcheck_get_captured_ts`, `osal_tmcheck_name_get_captured_ts` | **[owner]** — diagnostic-only paths intentionally unlocked. Data may be slightly inconsistent if racing against `_delete` |
| `osal_tmcheck_use`, `osal_tmcheck_avail` | **[any]** |

## Assert

| Macro | Notes |
|---|---|
| `OSAL_RUNTIME_ASSERT` | Safe to invoke from any thread; aborts the whole process on failure |
| `OSAL_STATIC_ASSERT` | Compile-time only |
