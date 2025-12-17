# Quick Start Guide

## 🚀 Get Started in 3 Steps

### Step 1: Install libzip
```bash
# MSYS2 MinGW terminal
pacman -S mingw-w64-x86_64-libzip
```

### Step 2: Build
```bash
make
# or
build.bat
```

### Step 3: Use
```bash
# Encrypt
./build/mycrypt-cli encrypt myfile.txt mypassword

# Decrypt
./build/mycrypt-cli decrypt myfile.txt.enc mypassword
```

## 📋 Command Reference

### Hash Password
```bash
mycrypt-cli hash <password> [cost] [salt]

# Examples:
mycrypt-cli hash mypassword
mycrypt-cli hash mypassword 10
mycrypt-cli hash mypassword 10 mysalt
```

### Encrypt File
```bash
mycrypt-cli encrypt <filepath> <password> [output_file]

# Examples:
mycrypt-cli encrypt document.pdf secret123
mycrypt-cli encrypt data.txt pass output.enc
```

### Decrypt File
```bash
mycrypt-cli decrypt <filepath> <password> [output_file]

# Examples:
mycrypt-cli decrypt document.pdf.enc secret123
mycrypt-cli decrypt output.enc pass data.txt
```

## 🎯 Key Features

| Feature | Description |
|---------|-------------|
| **Parallel Processing** | Automatic multi-core utilization |
| **Smart Chunking** | Adapts to file size (512KB - 8MB) |
| **Fast Password Check** | Rejects wrong password instantly |
| **ZIP Compression** | LZMA compression included |
| **Secure Random** | CryptGenRandom on Windows |

## 📊 Performance

| File Size | Chunk Size | Expected Speed |
|-----------|------------|----------------|
| < 5MB | No chunking | Similar to Python |
| 5-50MB | 512KB | 2-3x faster |
| 50-200MB | 2MB | 3-5x faster |
| > 200MB | 8MB | 5-10x faster |

## ⚠️ Common Issues

### "libzip not found"
```bash
# Install libzip first
pacman -S mingw-w64-x86_64-libzip
```

### "Wrong password"
```
Error code -2 means password is incorrect
Double-check your password
```

### Build fails
```bash
# Clean and rebuild
make clean
make
```

## 📚 Documentation

- **README.md** - Overview and features
- **SETUP.md** - Detailed installation
- **IMPLEMENTATION.md** - Technical details
- **ARCHITECTURE.md** - System design
- **SUMMARY.md** - Complete overview

## 🧪 Testing

```bash
# Run all tests
make test

# Run hash tests only
make test-hs

# Run encryption tests only
make test-en
```

## 💡 Tips

1. **Cost Factor:** Higher = more secure but slower (8-14 recommended)
2. **Large Files:** Automatically uses parallel processing
3. **Compression:** Built-in LZMA compression reduces file size
4. **Compatibility:** Can decrypt Python-encrypted files

## 🔒 Security Notes

- Uses cryptographically secure random salt
- Password never stored in plaintext
- Hash of hash for fast verification
- Metadata encryption prevents tampering

## 📞 Need Help?

1. Check error message and return code
2. Review SETUP.md for dependencies
3. Run test_encryption.bat to verify installation
4. Check ARCHITECTURE.md for technical details

---

**Ready to encrypt!** 🔐
