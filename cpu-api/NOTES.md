# cpu-api — Process API notes

Book chapter: [Process API](http://www.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)

Build all local exercises:

```sh
make
```

Clean:

```sh
make clean
```

---

## fork-variable

- **Goal:** See that after `fork()`, parent and child each get their own copy of variables (logical independence; physically often CoW).
- **Run:** `./fork-variable.out`
- **Observe:** Both sides print the same virtual address for `x`, but after assignment each has its own value (`200` vs `300`).
- **AI involvement:** assisted first draft; re-read for CoW meaning.

## wait-test

- **Goal:** Understand `wait()` / exit status; what happens if the child calls `wait()`.
- **Run:** `./wait-test.out`
- **Observe:** Parent successfully waits and reads exit code `42`. Child’s `wait()` fails (no children) with an error.
- **AI involvement:** assisted first draft.

## pipe-intro

- **Goal:** Use a pipe as a simple signal so parent prints `goodbye` only after child prints `hello`.
- **Run:** `./pipe-intro.out`
- **Observe:** Parent blocks on `read` until child writes one byte; unused ends are closed.
- **AI involvement:** assisted first draft.

## pipe

- **Goal:** Connect two processes like a shell pipeline: `ls | wc -l` via `pipe` + `dup2` + `execlp`.
- **Run:** `./pipe.out`
- **Observe:** Parent must close both pipe ends; otherwise `wc` may hang waiting for more input (no EOF).
- **AI involvement:** assisted first draft; pay special attention to close/`dup2` order when reviewing.

## concurrent-write-file

- **Goal:** Open a file before `fork()`, then parent and child both write; see shared file offset / interleaved output.
- **Run:** `./concurrent-write-file.out` then `cat data.input`
- **Observe:** Writes share the same open file description (offset advances for both); lines interleave. `data.input` is a runtime artifact (gitignored).
- **AI involvement:** assisted first draft.
