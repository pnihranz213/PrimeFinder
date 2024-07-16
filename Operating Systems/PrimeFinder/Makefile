# Set your compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Source files and target executables
SOURCES = main.c queue.c primefinder.c
EXECUTABLES = mlfScheduler primes

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target: build all executables
all: $(EXECUTABLES)

# Build the mlfScheduler executable
mlfScheduler: main.o queue.o
	$(CC) $(CFLAGS) $^ -o $@

# Build the primes executable
primes: primefinder.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean the project
clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

