# Setup Instructions

## Dependencies

This project requires **libzip** for ZIP compression functionality.

### Linux Setup (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install libzip-dev
```

### Linux Setup (Fedora/RHEL)

```bash
sudo dnf install libzip-devel
```

### Linux Setup (Arch)

```bash
sudo pacman -S libzip
```

### Windows Setup (MSYS2/MinGW)

1. Install MSYS2 from https://www.msys2.org/

2. Open MSYS2 MinGW 64-bit terminal and install libzip:
```bash
pacman -S mingw-w64-x86_64-libzip
```

3. Verify installation:
```bash
pkg-config --cflags --libs libzip
```

### Alternative: vcpkg

```bash
vcpkg install libzip:x64-windows
```

Then add to your build command:
```bash
-I[vcpkg]/installed/x64-windows/include -L[vcpkg]/installed/x64-windows/lib
```

## Build

```bash
make
```

## Features Implemented

✅ **Byte Manipulation Functions**
- rotate_left: Bitwise left rotation
- rotate_right: Bitwise right rotation  
- XOR encryption with key cycling
- byte_manipulations: Multi-round encryption with rotation + XOR
- byte_manipulations_reverse: Decryption counterpart

✅ **Intelligent Chunk Sizing**
- < 5MB files: No chunking (entire file)
- < 50MB files: 512KB chunks
- < 200MB files: 2MB chunks
- ≥ 200MB files: 8MB chunks

✅ **Parallel Processing**
- Automatic thread detection (hardware_concurrency)
- Parallel chunk encryption
- Thread-safe ZIP writing

✅ **Password Verification**
- Stores hash(hash(password)) with same salt/cost
- Fast password verification before decryption
- Prevents unnecessary decryption attempts

✅ **ZIP Compression**
- LZMA compression (best compression)
- Metadata storage (filename, salt, cost, hash_verify)
- Encrypted metadata verification
- Chunked file storage

## Usage

```bash
# Encrypt file (creates .enc file with ZIP compression)
mycrypt-cli encrypt file.txt mypassword

# Decrypt file
mycrypt-cli decrypt file.txt.enc mypassword

# Hash password
mycrypt-cli hash mypassword 10 mysalt
```
