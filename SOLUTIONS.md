# Personal OSTEP Solutions

This repository is based on the official OSTEP homework collection:

- Upstream: https://github.com/remzi-arpacidusseau/ostep-homework
- Book: [Operating Systems: Three Easy Pieces](https://pages.cs.wisc.edu/~remzi/OSTEP/)

Official simulators and READMEs stay as upstream provides them. My own C exercises, notes, and progress live alongside, mainly under each chapter directory.

## Layout

| Path | Purpose |
|------|---------|
| `upstream` remote | Official homework (fetch-only) |
| `origin` remote | This public solutions repo |
| `solutions` branch | Working branch for my answers and notes |
| `PROGRESS.md` | Chapter checklist |
| `<chapter>/NOTES.md` | Per-chapter review notes |
| `<chapter>/*.c` | My exercise programs |
| `<chapter>/Makefile` | Local build (`*.out`, gitignored) |

## Build

Example for Process API exercises:

```sh
cd cpu-api
make
./pipe.out
```

## Syncing official updates

```sh
git fetch upstream
git checkout master
git merge upstream/master
# then rebase or merge into solutions as needed
```

## Note on AI assistance

Some programs were drafted with AI help while reading the book. Notes under each chapter mark that so future review can focus on re-deriving the hard parts.
