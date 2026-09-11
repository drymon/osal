# dmosal documentation

Markdown documentation for the `dmosal` OS abstraction layer. Complements the
Doxygen-generated API reference (built via `make doc`).

| Document | What it covers |
|---|---|
| [architecture.md](architecture.md) | Layered design, subsystem overview, resource-manager pattern, callback threading model, backend model. |
| [api-spec.md](api-spec.md) | Behavioral specification per subsystem — functions, states, error codes, invariants. |
| [lifetime.md](lifetime.md) | Init/deinit ordering, resource ownership, when it is safe to use / release a resource. |
| [porting.md](porting.md) | How to add a new backend (e.g. an RTOS or bare-metal target). Reference: the POSIX backend. |
| [thread-safety.md](thread-safety.md) | Per-API table classifying every public call as thread-safe, single-owner, or init-thread-only. |

If you're just getting started, read them in this order:

1. `architecture.md` — the mental model
2. `thread-safety.md` — what you can and cannot call concurrently
3. `lifetime.md` — how resources come into existence and go away
4. `api-spec.md` — deeper look at each subsystem
5. `porting.md` — only if you're adding a new backend
