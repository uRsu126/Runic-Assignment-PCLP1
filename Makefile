# Copyright PCLP Team, 2025

# Compiler setup.
CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99

# Define targets, e.g., runic.
TARGETS=runic

# Manually define all targets.
build: $(TARGETS)

runic: runic.c
	$(CC) $(CFLAGS) runic.c -o runic -lm

# Clean the solution.
clean:
	rm -f $(TARGETS)

.PHONY: pack clean
