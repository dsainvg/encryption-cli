# mycrypt-cli

Terminal program for file encryption/decryption with custom password hashing algorithm.

## Features

- Custom iterative password hashing with configurable cost factor
- Cryptographically secure random salt generation (Windows CryptGenRandom)
- Deterministic hashing when salt is provided
- Advanced file encryption with:
  - Byte manipulation (rotate + XOR)
  - Intelligent chunk sizing based on file size
  - Parallel processing for large files
  - ZIP compression (LZMA)
  - Password verification (hash of hash)
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

## Dependencies

- libzip (for ZIP compression)

See [docs/SETUP.md](docs/SETUP.md) for installation instructions.

## Build

```bash
make
```

See [BUILD.md](BUILD.md) for detailed build instructions.

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
# Run all unit tests (190 tests: 80 hash + 110 encryption)
make test

# Run hash tests only (80 tests)
make test-hs

# Run encryption tests only (110 tests)
make test-en

# Run executable integration tests (15 tests)
chmod +x tests.sh
./tests.sh
```

## Algorithm

Custom password hashing algorithm matching the Python/C++ implementation:
- Multi-stage hash generation (hash1, hash2, hash3, hash4)
- Custom byte sorting via nth-element removal
- Latin-1 to UTF-8 encoding conversion
- Memoization for iterative strengthening
- Output format: `$salt$/$hash` or `$salt$/$hash2$hash4`

## Documentation

Detailed documentation available in [docs/](docs/) folder:
- [QUICKSTART.md](docs/QUICKSTART.md) - Quick start guide
- [SETUP.md](docs/SETUP.md) - Installation instructions
- [IMPLEMENTATION.md](docs/IMPLEMENTATION.md) - Technical details
- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - System design
