# Behavioral / API specification

Per-subsystem contract: what each function does, what it returns, and what
invariants it maintains. Complements the Doxygen-generated header
reference; see [thread-safety.md](thread-safety.md) for concurrency rules
and [lifetime.md](lifetime.md) for ordering rules.

## Error model

All error-returning functions use `osal_error_t` (see `osal_error.h`):

| Code | Meaning |
|---|---|
| `OSAL_E_OK` | Success. |
| `OSAL_E_PARAM` | An argument was NULL / out of range / otherwise invalid. |
| `OSAL_E_NOINIT` | The subsystem was not initialized. |
| `OSAL_E_RESRC` | The corresponding resource pool is exhausted. |
| `OSAL_E_FAILURE` | Unspecified failure. |
| `OSAL_E_OSCALL` | Underlying OS call failed. |
| `OSAL_E_TIMEOUT` | Waiting operation timed out. |
| `OSAL_E_QFULL` | Queue is full (send would block). |
| `OSAL_E_QEMPTY` | Queue is empty (receive would block). |
| `OSAL_E_INUSE` | Resource is already in use / already registered. |

Functions returning a pointer (`osal_*_create`) return `NULL` on failure.
Functions returning `void` are contract-guaranteed to succeed given valid
input.

`osal_errstr(e)` decodes any code to a static null-terminated string.

## Top-level

`osal_init(osal_config_t *config)` — set up every subsystem. Pass `NULL`
for defaults (stdout logger, INFO level for the internal `osal` module).
Idempotent: a second call while already initialized returns `OSAL_E_OK`
without touching state.

`osal_deinit(void)` — tear down every subsystem in reverse order. Idempotent
in the "not initialized" case.

`osal_version(void)` — returns a static string.

`osal_print_resource(void)` — dumps used/total counts for every pool to
the log module.

## Mutex

State: `{ uninitialized, initialized-empty, initialized-with-mutexes-alive,
uninitialized }`.

- `osal_mutex_init(void)` — sets up the mutex pool. Safe to call more than
  once; subsequent calls are no-ops.
- `osal_mutex_deinit(void)` — releases the mutex pool. All mutexes must
  have been deleted first.
- `osal_mutex_create(void)` — allocates a mutex; returns NULL if not
  initialized or pool exhausted.
- `osal_mutex_delete(mutex)` — releases a mutex back to the pool. Behavior
  undefined if any thread holds or is waiting on the mutex.
- `osal_mutex_lock(mutex)` / `osal_mutex_unlock(mutex)` — non-recursive.
  Returns `OSAL_E_PARAM` on NULL, `OSAL_E_OSCALL` on backend failure (a
  real failure aborts via assert).
- `osal_mutex_use()` / `osal_mutex_avail()` — pool statistics.

## Semaphore

State machine: counter ≥ 0. `post` increments, `wait` decrements-or-blocks.

- `osal_sem_init(mutex)` — takes the shared mutex for pool protection.
- `osal_sem_deinit(void)` — tear down.
- `osal_sem_create(void)` — allocates a semaphore initialized with count 0.
- `osal_sem_delete(sem)` — releases. No thread may be blocked in wait.
- `osal_sem_post(sem)` — increments the counter (unblocks one waiter).
- `osal_sem_wait(sem)` — blocks until counter > 0, then decrements. No
  timeout.
- `osal_sem_waittime(sem, usec)` — like `wait` but returns `OSAL_E_TIMEOUT`
  after `usec` microseconds.

## Task

- `osal_task_init(mutex)` — pool init.
- `osal_task_deinit(void)` — pool tear-down. Does NOT stop running tasks.
- `osal_task_create(cfg)` — allocates a task slot and starts the handler
  immediately on a new thread. `cfg->task_handler` must be non-NULL. Hints
  in `cfg` (`stack_addr`, `stack_size`, `priority`, `name`) are
  backend-dependent and may be ignored.
- `osal_task_delete(task)` — stops the task if still running, joins the
  thread, releases the slot. Safe on a task whose handler has already
  returned.
- `osal_task_use()` / `osal_task_avail()`.

The task handler signature: `void (*)(void *arg)`. When it returns, the
underlying thread exits and the slot remains "in use" until
`osal_task_delete` is called.

## Timer

- `osal_timer_init(mutex)` — pool init.
- `osal_timer_deinit(void)` — tear-down. Callers must have deleted all
  active timers.
- `osal_timer_create(expire, arg)` — allocates a timer. The `expire`
  callback is invoked from a backend-managed context (not the caller's
  thread). `expire` must be non-NULL.
- `osal_timer_start(timer, usec, repeat)` — arm the timer. First
  expiration in `usec` microseconds. If `repeat`, re-arms with the same
  interval after each expiration. `usec == 0` returns `OSAL_E_PARAM`.
- `osal_timer_stop(timer)` — disarm. Idempotent.
- `osal_timer_delete(timer)` — release. Asserts if called while the
  callback is currently executing (from any thread).

## Queue

- `osal_queue_init(mutex)` — pool init.
- `osal_queue_deinit(void)` — tear-down.
- `osal_queue_create(cfg)` — allocates and opens or attaches a named
  queue. `cfg->name`, `cfg->msglen`, `cfg->qsize` must all be non-empty /
  non-zero. Attaching to an existing kernel queue with mismatched size
  fails.
- `osal_queue_delete(queue)` — close. If this handle originally created
  the underlying kernel queue, also unlinks it.
- `osal_queue_send(queue, msg, msglen)` — non-blocking. Returns
  `OSAL_E_QFULL` if the queue is full.
- `osal_queue_recv(queue, buf, bufsize, timeout_usec)` — blocks up to
  `timeout_usec`. Returns `OSAL_E_TIMEOUT` on deadline elapse,
  `OSAL_E_QEMPTY` on empty wake (rare).

## Time

- `osal_sleep(sec)` / `osal_usleep(microsec)` — thread sleep. Ignore signal
  interruption.
- `osal_clock_time(&nsec)` — monotonic clock in nanoseconds. Suitable for
  measuring intervals; not a wall-clock timestamp.

## Log

The log subsystem indexes log modules by integer, letting the application
route different modules to different levels at runtime. Module index 0 is
reserved for the library itself (`OSAL_LOG_MODULE_INDEX`).

- `osal_log_init(log_output)` — set the output function. `NULL` returns
  `OSAL_E_PARAM`. Subsequent calls (while already initialized) are no-ops.
- `osal_log_deinit(void)` — clears output + module table.
- `osal_log_module_init(idx, name, level, ts)` — reserve a module slot.
  `ts=true` prepends a timestamp to every message from this module.
  Returns `OSAL_E_INUSE` if the slot is already registered.
- `osal_log_module_change(idx, level)` — cheap live level change (or
  `OSALOG_LEVEL_NONE` to silence).
- `osal_log_print(idx, ts, level, format, ...)` — usually called via the
  `OSALOG_*` macros, which resolve to a per-source-file `OSALOG_MODULE`
  index.

Compile-time gating: define `OSALOG_BUILD_LEVEL` before including the
library headers to compile out anything more verbose than that level.

## Resource manager

Public but rarely called directly by application code (subsystems use it
internally). The `OSAL_RM_USEROBJMAN_DECLARE` / `_INIT` macros bundle the
common pattern.

- `osal_rm_init(rm, cfg)` — with `cfg->mutex == NULL`, the RM is not
  thread-safe.
- `osal_rm_deinit(rm)` — resets the LIFO. Locks its mutex if it has one,
  so the mutex must still be valid at this call.
- `osal_rm_alloc(rm)` / `osal_rm_free(rm, resrc)` — pool ops.
- `osal_rm_use(rm)` / `osal_rm_avail(rm)`.

## LIFO

Intrusive stack over `osal_lifo_node_t`. Not internally synchronized
(a plain linked list, no atomics); callers synchronize externally if the
LIFO is shared. Used by `osal_rm` as the free-list backing.

## Tmcheck

Time-checkpoint utility for micro-benchmarking hot paths. See the
`osal_tmcheck.h` header for details. `osal_tmcheck_capture_ts` records
"first-hit only" — subsequent captures on the same slot are no-ops until
`osal_tmcheck_reset` is called. Useful for capturing "first time this
line ran" rather than "every time this line ran".

## Assert

`OSAL_RUNTIME_ASSERT(cond)` — if `cond` is false, print
`assert:<file>:<line>!!!` and abort. Can be overridden by defining
`OSAL_RUNTIME_ASSERT` before including `osal_assert.h` (e.g. to redirect
into a custom fault handler).

`OSAL_STATIC_ASSERT(cond)` — compile-time; usable at file scope. Not
usable more than once per translation unit with different `cond` values
that would yield different array sizes.
