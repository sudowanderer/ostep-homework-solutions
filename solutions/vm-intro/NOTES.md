# vm-intro — Address Spaces notes

Book chapter: [Address Spaces](http://www.cs.wisc.edu/~remzi/OSTEP/vm-intro.pdf)

This directory: personal exercise only (`memory-user`).

## memory-user

- **Goal:** Allocate a user-specified amount of memory and keep touching each
  page so the process stays resident. Use OS tools to watch virtual vs.
  physical memory while reading the chapter on address spaces.
- **Build:**

```sh
cd solutions/vm-intro
cc -Wall -o memory-user memory-user.c
```

- **Run:**

```sh
./memory-user 100          # allocate ~100 MB
```

Program prints its PID, then loops forever (Ctrl-C to stop).

- **Observe (macOS):**

```sh
# RSS = resident set (physical pages in use), VSZ = virtual size
ps -p <pid> -o pid,rss,vsz,command

# or sample repeatedly
while true; do ps -p <pid> -o rss=; sleep 1; done
```

- **Observe (Linux):** same idea with `ps`, or `cat /proc/<pid>/status`
  (`VmSize`, `VmRSS`).

- **What to notice:**
  - After `malloc` alone, RSS may still be small (lazy allocation / demand
    paging): pages are not always mapped until first touch.
  - After the touch loop runs, RSS climbs toward the requested size.
  - VSZ is typically larger than RSS (virtual address space reservation vs.
    pages actually backed by RAM).
  - Touching one byte every 4096 bytes is enough to fault in each page.

- **AI involvement:** assisted first draft; re-read for demand paging vs. RSS.
