# Compiler settings
CC = gcc  # Default compiler for native builds
CROSS_COMPILE ?=  # Cross-compilation prefix (optional)

# Target executable name
TARGET = writer

# Source files
SRC = writer.c

# Compiler flags
CFLAGS = -Wall -Wextra

# Default target: Build the writer application
all: $(TARGET)

# Rule to compile the writer application
$(TARGET): $(SRC)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $<

# Clean target: Remove compiled files
clean:
	rm -f $(TARGET) *.o

# Phony targets: Prevents make from treating them as files
.PHONY: all clean