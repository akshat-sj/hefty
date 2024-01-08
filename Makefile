CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lpsapi
TARGET = main.exe

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)