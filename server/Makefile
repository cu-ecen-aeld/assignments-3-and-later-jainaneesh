#$(shell read -n 1 -s -r -p "Paused for debugging. Execution shifted to aesd-assignments.mk Press any key to continue..."; echo)
# Compiler settings
CROSS_COMPILE ?=  # Cross-compilation prefix (optional)
CC = $(CROSS_COMPILE)gcc  # Default compiler for native builds
$(info CROSS_COMPILE=$(CROSS_COMPILE))
# Target executable name
TARGET = aesdsocket

# Source files
SRC = aesdsocket.c

# Compiler flags
CFLAGS = -Wall -Wextra

# Default target: Build the writer application
all: $(TARGET)

# Rule to compile the writer application
$(TARGET): $(SRC)
	$(info *** COMPILATION COMMAND: $(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $< ***)
	$(error "Paused for debugging. Execution shifted to aesd-assignments.mk Press any key to continue..."; echo)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $<

# Clean target: Remove compiled files
clean:
	rm -f $(TARGET) *.o

# Phony targets: Prevents make from treating them as files
.PHONY: all clean
