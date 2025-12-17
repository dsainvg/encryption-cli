CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude

SRC = src/main.c src/cli.c src/crypto.c src/utils.c
OBJ = build/obj/main.o build/obj/cli.o build/obj/crypto.o build/obj/utils.o
EXE = build/mycrypt-cli.exe
TEST_OBJ = build/obj/test_crypto.o build/obj/crypto.o build/obj/utils.o
TEST_EXE = build/test_crypto.exe

all: build/obj $(EXE)

build/obj:
	if not exist build mkdir build
	if not exist build\obj mkdir build\obj

build/obj/%.o: src/%.c include/*.h
	$(CC) $(CFLAGS) -c $< -o $@

build/obj/test_crypto.o: tests/test_crypto.c include/*.h
	$(CC) $(CFLAGS) -c tests/test_crypto.c -o $@

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(TEST_EXE): $(TEST_OBJ)
	$(CC) $(CFLAGS) $(TEST_OBJ) -o $@

test: $(TEST_EXE)
	$(TEST_EXE)

clean:
	if exist build rmdir /s /q build

.PHONY: all test clean
