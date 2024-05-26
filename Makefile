CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = vm
SRCS = vm.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) correct.txt