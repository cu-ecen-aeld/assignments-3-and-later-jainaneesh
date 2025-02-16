# Compiler settings
CC ?= $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -g
#CROSS_COMPILE ?=
TARGET = aesdsocket
SRC = aesdsocket.c
OBJ = $(SRC:.c=.o)


# Default target
all: $(TARGET)

default: all

# Compile the program
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Clean compiled files
clean:
	rm -f $(TARGET) $(OBJ)

# Install to system (optional)
install: $(TARGET)
	sudo cp $(TARGET) /usr/bin
	sudo chmod +x /usr/bin/$(TARGET)

# Uninstall
uninstall:
	sudo rm -f /usr/bin/$(TARGET)

# Run the server
run:
	$(TARGET)

# Run in daemon mode
daemon:
	$(TARGET) -d

# Cross-compilation example:
# make CROSS_COMPILE=arm-linux-gnueabi-
.PHONY: all clean install uninstall run daemon default
