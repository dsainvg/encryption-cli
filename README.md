# mycrypt-cli

Terminal program for file encryption/decryption with custom password hashing algorithm.

## Features

- Custom iterative password hashing with configurable cost factor
- Cryptographically secure random salt generation (Windows CryptGenRandom)
- Deterministic hashing when salt is provided
- XOR-based file encryption/decryption
- Cost factor 8-14 recommended (256 to 16384 iterations)

## Usage

```bash
# Hash password with default cost (10 = 1024 iterations)
mycrypt-cli hash <password>

# Hash with custom cost and salt
mycrypt-cli hash <password> [cost] [salt]

# Encrypt/decrypt files
mycrypt-cli encrypt <filepath> <password> [output_file]
mycrypt-cli decrypt <filepath> <password> [output_file]
```

## Build

```bash
make
# Or manually:
gcc -Wall -Wextra -O2 -Iinclude -c src/*.c
gcc *.o -ladvapi32 -o mycrypt-cli.exe
```

## Examples

```bash
# Hash with random salt
./build/mycrypt-cli.exe hash mypassword

# Hash with fixed salt (deterministic)
./build/mycrypt-cli.exe hash mypassword 10 fixedSalt123456

# Encrypt file
./build/mycrypt-cli.exe encrypt file.txt mypassword

# Decrypt file
./build/mycrypt-cli.exe decrypt file.txt.enc mypassword
```

## Test

```bash
# Run comprehensive hash tests (30 tests)
cmd /c hash_tests.bat
```

## Algorithm

Custom password hashing algorithm matching the Python/C++ implementation:
- Multi-stage hash generation (hash1, hash2, hash3, hash4)
- Custom byte sorting via nth-element removal
- Latin-1 to UTF-8 encoding conversion
- Memoization for iterative strengthening
- Output format: `$salt$/$hash` or `$salt$/$hash2$hash4`
