CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -Iinclude

SRC = src/main.cpp src/cli.cpp src/crypto.cpp src/utils.cpp
OBJ = build/obj/main.o build/obj/cli.o build/obj/crypto.o build/obj/utils.o
EXE = build/mycrypt-cli.exe
TEST_OBJ = build/obj/test_crypto.o build/obj/crypto.o build/obj/utils.o
TEST_EXE = build/test_crypto.exe

all: build/obj $(EXE)

build/obj:
	if not exist build mkdir build
	if not exist build\obj mkdir build\obj

build/obj/%.o: src/%.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/test_crypto.o: tests/test_crypto.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c tests/test_crypto.cpp -o $@

$(EXE): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -ladvapi32 -o $@

$(TEST_EXE): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $(TEST_OBJ) -ladvapi32 -o $@

test: $(TEST_EXE)
	$(TEST_EXE)

clean:
	if exist build rmdir /s /q build

.PHONY: all test clean
