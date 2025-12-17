CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -Iinclude $(shell pkg-config --cflags libzip 2>/dev/null)

ifeq ($(OS),Windows_NT)
    LIBS = -ladvapi32 $(shell pkg-config --libs libzip 2>nul || echo -lzip)
    EXE_EXT = .exe
    RM = del /q
    MKDIR = if not exist
else
    LIBS = $(shell pkg-config --libs libzip 2>/dev/null || echo -lzip) -lpthread
    EXE_EXT = .exe
    RM = rm -f
    MKDIR = mkdir -p
endif

all: build/obj build/mycrypt-cli$(EXE_EXT)

build/obj:
	@mkdir -p build/obj

build/obj/main.o: src/main.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o build/obj/main.o

build/obj/cli.o: src/cli.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c src/cli.cpp -o build/obj/cli.o

build/obj/crypto.o: src/crypto.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c src/crypto.cpp -o build/obj/crypto.o

build/obj/utils.o: src/utils.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c src/utils.cpp -o build/obj/utils.o

build/obj/encryption.o: src/encryption.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c src/encryption.cpp -o build/obj/encryption.o

build/mycrypt-cli$(EXE_EXT): build/obj/main.o build/obj/cli.o build/obj/crypto.o build/obj/utils.o build/obj/encryption.o
	$(CXX) $(CXXFLAGS) build/obj/main.o build/obj/cli.o build/obj/crypto.o build/obj/utils.o build/obj/encryption.o $(LIBS) -o build/mycrypt-cli$(EXE_EXT)

build/obj/test_crypto.o: tests/test_crypto.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c tests/test_crypto.cpp -o build/obj/test_crypto.o

ifeq ($(OS),Windows_NT)
build/test_crypto$(EXE_EXT): build/obj/test_crypto.o build/obj/crypto.o build/obj/utils.o
	$(CXX) $(CXXFLAGS) build/obj/test_crypto.o build/obj/crypto.o build/obj/utils.o -ladvapi32 -o build/test_crypto$(EXE_EXT)
else
build/test_crypto$(EXE_EXT): build/obj/test_crypto.o build/obj/crypto.o build/obj/utils.o
	$(CXX) $(CXXFLAGS) build/obj/test_crypto.o build/obj/crypto.o build/obj/utils.o -o build/test_crypto$(EXE_EXT)
endif

build/obj/test_encryption.o: tests/test_encryption.cpp include/*.h
	$(CXX) $(CXXFLAGS) -c tests/test_encryption.cpp -o build/obj/test_encryption.o

ifeq ($(OS),Windows_NT)
build/test_encryption$(EXE_EXT): build/obj/test_encryption.o build/obj/encryption.o build/obj/crypto.o build/obj/utils.o
	$(CXX) $(CXXFLAGS) build/obj/test_encryption.o build/obj/encryption.o build/obj/crypto.o build/obj/utils.o -ladvapi32 -lzip -o build/test_encryption$(EXE_EXT)
else
build/test_encryption$(EXE_EXT): build/obj/test_encryption.o build/obj/encryption.o build/obj/crypto.o build/obj/utils.o
	$(CXX) $(CXXFLAGS) build/obj/test_encryption.o build/obj/encryption.o build/obj/crypto.o build/obj/utils.o -lzip -lpthread -o build/test_encryption$(EXE_EXT)
endif

test-hs: build/test_crypto$(EXE_EXT)
	./build/test_crypto$(EXE_EXT)

test-en: build/test_encryption$(EXE_EXT)
	./build/test_encryption$(EXE_EXT)

test: build/test_crypto$(EXE_EXT) build/test_encryption$(EXE_EXT)
	./build/test_crypto$(EXE_EXT)
	./build/test_encryption$(EXE_EXT)

clean:
	rm -rf build

.PHONY: all test test-hs test-en clean
