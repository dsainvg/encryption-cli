# Build Instructions

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential libzip-dev
```

### Linux (Fedora/RHEL)
```bash
sudo dnf install gcc-c++ libzip-devel
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-libzip
```

## Build

```bash
make
```

## Output

**Executable location:**
- `./build/mycrypt-cli.exe`

## Test

```bash
# Run all tests (190 tests)
make test

# Run hash tests only (80 tests)
make test-hs

# Run encryption tests only (110 tests)
make test-en
```

## Clean

```bash
make clean
```

## Usage

```bash
# Hash password
./build/mycrypt-cli.exe hash mypassword

# Encrypt file
./build/mycrypt-cli.exe encrypt file.txt mypassword

# Decrypt file
./build/mycrypt-cli.exe decrypt file.txt.enc mypassword
```

## Troubleshooting

### libzip not found
```bash
# Check if libzip is installed
pkg-config --libs libzip

# If not installed, install it (see Prerequisites)
```

### Compilation errors
```bash
# Clean and rebuild
make clean
make
```

### Test failures
```bash
# Run tests individually
make test-hs    # Hash tests
make test-en    # Encryption tests
```
