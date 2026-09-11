# Resource lifetime

`dmosal` is a static-allocation library — every resource comes from a
compile-time-sized pool. Understanding when resources come into existence,
when they can be used, and when they must be released is critical, because
the layer intentionally does very little runtime bookkeeping to catch
misuse.

## Program lifecycle

```
    [ startup ]
        |
        v
    osal_init(cfg)                         (1) library goes live
        |
        v
    < resources created — mutex/sem/task/timer/queue/tmcheck >
        |
        v
    < runtime: locks, sends, timer callbacks, task handlers, ... >
        |
        v
    < resources deleted (optional; ok to skip for singletons) >
        |
        v
    osal_deinit()                          (2) library goes dark
        |
        v
    [ shutdown ]
```

- Between (1) and (2), every subsystem is usable.
- After (2), no `osal_*` call is safe (except a subsequent `osal_init`
  that reinitializes the whole layer).

## `osal_init` — what happens

`osal_init(config)` initializes subsystems in this order:

1. `osal_mutex_init` — the mutex pool must come first because every other
   subsystem uses a shared mutex from that pool.
2. `osal_log_init(log_output)` — the log sink is set. Default writes to
   stdout. **After this point** the `OSALOG_*` macros produce output.
3. `osal_log_module_init(OSAL_LOG_MODULE_INDEX, "osal", level, false)`
   registers the library's internal logging module.
4. `osal_mutex_create()` allocates the shared mutex from the mutex pool.
   The library holds a private reference in `s_shared_mutex`.
5. `osal_sem_init(s_shared_mutex)`, then `osal_task_init(...)`,
   `osal_timer_init(...)`, `osal_queue_init(...)`, `osal_tmcheck_init(...)`
   — every pool gets the shared mutex so pool operations are serialized.

If any step fails, the library aborts via `OSAL_RUNTIME_ASSERT`. In
practice the underlying subsystems only fail when the corresponding pool
is misconfigured (size 0), so this rarely fires.

## `osal_deinit` — order matters

Tear-down is the reverse of init, and the **order is significant** because
the subsystem pools reference the shared mutex:

1. `osal_sem_deinit`, `osal_task_deinit`, `osal_timer_deinit`,
   `osal_queue_deinit`, `osal_tmcheck_deinit` — each of these calls
   `osal_rm_deinit(...)` which locks the shared mutex.
2. `osal_log_deinit` — independent of the shared mutex.
3. `osal_mutex_delete(s_shared_mutex)` — return the shared mutex to its
   pool. **This must come after step 1** — deleting the shared mutex
   first would leave the subsystems' `osal_rm_deinit` locking a destroyed
   mutex. (An early version of the library had this bug.)
4. `osal_mutex_deinit` — tear down the mutex pool itself.

**Rule of thumb**: only call `osal_deinit` once the application has
quiesced. In particular:

- Every task handler must have finished (delete tasks first, or design
  them as one-shots that self-terminate).
- Every timer callback must have finished (`osal_timer_stop` then wait).
- No thread may be inside `osal_queue_send/recv`, `osal_sem_wait`,
  `osal_mutex_lock`, etc.

`dmosal` does not attempt to force this — callers are responsible for
serializing shutdown against runtime activity.

## Per-resource lifecycle

### Mutex

```
create -> [locked/unlocked N times] -> delete
```

`osal_mutex_create()` returns a pointer valid until the matching
`osal_mutex_delete()`. No thread may hold or attempt to lock the mutex
during delete — the library does not check this.

### Semaphore

```
create -> [posted N times / waited on N times] -> delete
```

Same rule: no thread may be blocked inside `osal_sem_wait` /
`osal_sem_waittime` when `osal_sem_delete` runs.

### Task

```
create --spawns--> [task_handler running on its own thread] --returns-->
                                                                 |
                              delete (pthread_cancel + join) <---+
```

- `osal_task_create` starts the handler immediately.
- `osal_task_delete` cancels the thread (if still running) and joins it.
- Deleting a task whose handler has already returned normally is safe —
  cancel is a no-op, join reaps the exit status.

### Timer

```
create -> [started/stopped, callback fires 0+ times] -> stop -> delete
```

- `osal_timer_delete` **must not** be called while the callback is
  running. The library detects and aborts on this — see
  [thread-safety.md](thread-safety.md).
- Recommended pattern: `osal_timer_stop(t)`; ensure any in-flight
  callback has returned; `osal_timer_delete(t)`.

### Queue

```
create --opens or attaches--> [send/recv N times] --unlinks if creator--> delete
```

- On the POSIX backend, `osal_queue_create` may either create the
  underlying kernel queue or attach to an existing one with the same
  name. `osal_queue_delete` unlinks the kernel queue only when this
  handle originally created it.
- Attach + mismatched configuration fails at create time.

### Tmcheck

```
create -> [capture_ts / reset / print] -> delete
```

Diagnostic-only. Concurrent create/delete are internally serialized;
concurrent capture/print/get with a delete may see inconsistent state
(deliberate design choice — see [thread-safety.md](thread-safety.md)).

## Static-vs-dynamic ownership

The library favors an **init-then-run** pattern: allocate every resource
you'll ever need during `osal_init` + a few `_create` calls, then run
without further pool ops.

- This makes lifetime analysis local: each resource is created once,
  used until shutdown, and (optionally) deleted at the very end.
- It also matches the deferred `osal_lock_runtime()` design (see
  [architecture.md](architecture.md)) where creates and deletes are
  forbidden outside the init phase.

Dynamic create/delete during operation is legal — the pools are
thread-safe — but it makes lifetime reasoning harder and is not the
intended primary use case.

## What the library will NOT do for you

- Detect use-after-delete on a resource handle.
- Detect double-delete on the same handle (in most subsystems). Tmcheck
  is the one exception — it guards `osal_tmcheck_delete` against
  double-decrement.
- Cancel outstanding timer callbacks on `osal_deinit`.
- Reap task threads on `osal_deinit` if you skipped `osal_task_delete`.

These are the caller's responsibility. In practice: delete resources in
the reverse order you created them, and only call `osal_deinit` when
nothing else is running.
