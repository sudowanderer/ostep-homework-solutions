# Personal OSTEP Solutions

This repository is based on the official OSTEP homework collection:

- Upstream: https://github.com/remzi-arpacidusseau/ostep-homework
- Book: [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/)

**All of my own files live under [`solutions/`](solutions/).**  
Official simulators and chapter READMEs stay in the original chapter directories and are not mixed with answers.

## Layout

| Path | Purpose |
|------|---------|
| `upstream` remote | Official homework (fetch-only) |
| `origin` remote | This public solutions repo |
| `solutions` branch | Working branch for my answers and notes |
| Chapter dirs (`cpu-api/`, …) | **Official only** — simulators, READMEs |
| [`solutions/`](solutions/) | **Everything I created** — filter this folder to review |
| `solutions/PROGRESS.md` | Chapter checklist |
| `solutions/<chapter>/*.c` | Exercise programs |
| `solutions/<chapter>/NOTES.md` | Per-chapter review notes |
| `solutions/<chapter>/Makefile` | Local build (`*.out`, gitignored) |

## Browse only my work

```sh
find solutions -type f | sort
```

On GitHub: open the [`solutions/`](https://github.com/sudowanderer/ostep-homework-solutions/tree/solutions/solutions) directory.

## Build

```sh
cd solutions/cpu-api
make
./pipe.out
```

## Syncing official updates

```sh
git fetch upstream
git checkout master
git merge upstream/master
# then merge or rebase into solutions as needed
```

## Note on AI assistance

Some programs were drafted with AI help while reading the book. Notes under each chapter mark that so future review can focus on re-deriving the hard parts.
