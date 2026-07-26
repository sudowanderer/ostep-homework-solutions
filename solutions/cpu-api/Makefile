CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = \
	fork-variable.c \
	wait-test.c \
	pipe-intro.c \
	pipe.c \
	concurrent-write-file.c

BINS = $(SRCS:.c=.out)

.PHONY: all clean

all: $(BINS)

%.out: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINS)
