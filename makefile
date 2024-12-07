CC = gcc
CFLAGS = -Wall -Wextra -O3 -flto -march=native -Iinclude
TARGET = process_pdb
SRC = src/process_pdb.c

$(TARGET): $(SRC) include/process_pdb.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
