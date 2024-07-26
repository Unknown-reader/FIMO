TARGET = fimo

CC = gcc

CFLAGS = -I/usr/include -Iinclude -static

LDFLAGS = -L/usr/lib -lssl -lcrypto -ldl -pthread

SRC = src/main.c src/dir-file-tools.c src/sha256.c src/integrity-tools.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
