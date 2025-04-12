#$(shell read -n 1 -s -r -p "Paused for debugging. Execution shifted to aesd-assignments.mk Press any key to continue..."; echo)
# Compiler settings
CROSS_COMPILE ?=  # Cross-compilation prefix (optional)
CC ?= $(CROSS_COMPILE)gcc  # Default compiler for native builds
$(info CROSS_COMPILE=$(CROSS_COMPILE))
# Target executable name
TARGET = aesdsocket

# Source files
SRC = aesdsocket.c

# Compiler flags
CFLAGS = -Wall -Wextra
LDFLAGS = -lpthread

# Default target: Build the writer application
all: $(TARGET)

# Rule to compile the writer application
$(TARGET): $(SRC) FORCE
	$(info *** COMPILATION COMMAND: $(CC) $(CFLAGS) -o $@ $< ***)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean target: Remove compiled files
clean:
	rm -f $(TARGET) *.o

# Phony targets: Prevents make from treating them as files
.PHONY: all clean

FORCE: #This ensures the target is always rebuilt
