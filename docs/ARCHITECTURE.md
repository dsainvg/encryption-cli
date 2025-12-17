# Architecture Overview

## Module Structure

```
mycrypt-cli/
├── include/
│   ├── cli.h           # Command-line argument parsing
│   ├── crypto.h        # Password hashing (existing)
│   ├── encryption.h    # NEW: Advanced encryption functions
│   └── utils.h         # Utility functions
│
├── src/
│   ├── main.cpp        # Entry point, CLI handling
│   ├── cli.cpp         # Argument parsing implementation
│   ├── crypto.cpp      # Custom hash algorithm
│   ├── encryption.cpp  # NEW: Byte manipulation + file encryption
│   └── utils.cpp       # Utilities
│
└── tests/
    ├── hash_tests.bat       # Hash algorithm tests
    └── test_encryption.bat  # NEW: Encryption tests
```

## Data Flow

### Encryption Flow

```
Input File
    ↓
[Read File Size] → [Determine Chunk Size]
    ↓                    ↓
[Generate Salt] ← [Intelligent Sizing]
    ↓
[Hash Password with Salt/Cost]
    ↓
[Create Hash of Hash] ← [Password Verification]
    ↓
[Create ZIP Archive]
    ↓
[Write Metadata] → filedata.crypt (plaintext)
    ↓              → filedata_enc.crypt (encrypted)
    ↓
[Read File in Chunks] ← [Parallel Processing]
    ↓
[For Each Chunk]:
    ├─ [Split into 1KB sub-chunks]
    ├─ [byte_manipulations(sub_chunk, hash, chunk_index)]
    │   ├─ rotate_left(data, k)
    │   └─ xor_bytes(data, key)
    └─ [Write to ZIP] → filedata_chunk_N.crypt
    ↓
[Close ZIP Archive]
    ↓
Output: encrypted.enc
```

### Decryption Flow

```
Input: encrypted.enc
    ↓
[Open ZIP Archive]
    ↓
[Read Metadata] ← filedata.crypt
    ↓
[Parse: filename, salt, cost, hash_verify]
    ↓
[Hash Password with Salt/Cost]
    ↓
[Verify Hash of Hash] ← [Fast Password Check]
    ↓                     (Fail fast if wrong)
[Read Encrypted Metadata] ← filedata_enc.crypt
    ↓
[Decrypt & Verify Metadata]
    ↓
[List All Chunks] → Sort by index
    ↓
[For Each Chunk]:
    ├─ [Read from ZIP]
    ├─ [Split into 1KB sub-chunks]
    ├─ [byte_manipulations_reverse(sub_chunk, hash, chunk_index)]
    │   ├─ xor_bytes(data, key)
    │   └─ rotate_right(data, k)
    └─ [Write to Output File]
    ↓
[Close ZIP Archive]
    ↓
Output: decrypted file
```

## Byte Manipulation Algorithm

```
byte_manipulations(data, key, iteration):
    iterat = 10 + (iteration % 6)  // 10-15 rounds
    n = key[(key[1] + key[2]) % (len(key) - 5)] % 7
    
    for i in 0..iterat:
        rotate_left(data, 1 + ((n + i) % 7))  // 1-7 bits
        xor_bytes(data, key)
    
    return data

byte_manipulations_reverse(data, key, iteration):
    // Same parameters, reverse order
    for i in (iterat-1)..0:
        xor_bytes(data, key)
        rotate_right(data, 1 + ((n + i) % 7))
    
    return data
```

## Parallel Processing Strategy

```
File → [Split into Chunks]
         ↓
    [Chunk Queue]
         ↓
    ┌────┴────┬────────┬────────┐
    ↓         ↓        ↓        ↓
[Thread 1] [Thread 2] [Thread 3] [Thread N]
    ↓         ↓        ↓        ↓
[Encrypt]  [Encrypt] [Encrypt] [Encrypt]
    ↓         ↓        ↓        ↓
    └────┬────┴────────┴────────┘
         ↓
    [Mutex Lock]
         ↓
    [Write to ZIP]
```

## Memory Management

### Encryption
- Read chunks on-demand (not entire file)
- Process in parallel batches
- Free memory after ZIP write
- Adaptive chunk sizing reduces memory footprint

### Decryption
- Read chunks from ZIP sequentially
- Decrypt and write immediately
- No need to load entire file
- Streaming approach for large files

## Security Layers

```
Layer 1: Password Hashing
    ├─ Custom iterative algorithm
    ├─ Configurable cost (2^cost iterations)
    └─ Cryptographically secure salt

Layer 2: Hash Verification
    ├─ Hash of hash stored in metadata
    ├─ Fast password rejection
    └─ No timing attacks

Layer 3: Metadata Encryption
    ├─ Encrypted with hashed password
    ├─ Verification before decryption
    └─ Prevents tampering

Layer 4: File Encryption
    ├─ Byte manipulation (rotate + XOR)
    ├─ Chunk-based with iteration index
    └─ 1KB sub-chunks for diffusion

Layer 5: Compression
    ├─ ZIP with LZMA compression
    ├─ Reduces file size
    └─ Additional obfuscation
```

## Performance Characteristics

### Time Complexity
- **Hashing:** O(2^cost × password_length)
- **Encryption:** O(file_size / num_threads)
- **Decryption:** O(file_size / num_threads)

### Space Complexity
- **Memory:** O(chunk_size × num_threads)
- **Disk:** O(file_size × compression_ratio)

### Scalability
- Linear scaling with CPU cores
- Constant memory usage (chunk-based)
- Suitable for files of any size

## External Dependencies

```
libzip
├─ Purpose: ZIP archive creation/extraction
├─ Features: LZMA compression, streaming
└─ Platform: Cross-platform (Windows/Linux/macOS)

advapi32 (Windows)
├─ Purpose: CryptGenRandom
├─ Features: Cryptographically secure random
└─ Platform: Windows only

C++ Standard Library
├─ <thread>: Parallel processing
├─ <mutex>: Thread synchronization
├─ <fstream>: File I/O
└─ <vector>: Dynamic arrays
```

## Error Handling

```
Return Codes:
    0  : Success
   -1  : File I/O error, ZIP error, or general failure
   -2  : Wrong password (decryption only)

Error Flow:
    Error Detected
        ↓
    [Free Resources]
        ↓
    [Close Files/ZIP]
        ↓
    [Return Error Code]
        ↓
    [Main: Print Error Message]
```
