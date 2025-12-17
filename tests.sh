#!/bin/bash

EXE=./build/mycrypt-cli.exe
PASSED=0
FAILED=0

echo "========================================"
echo "mycrypt-cli Executable Tests"
echo "========================================"
echo

# Check if executable exists
if [ ! -f "$EXE" ]; then
    echo "ERROR: $EXE not found!"
    echo "Please run: make"
    exit 1
fi

echo "Testing executable: $EXE"
echo

# Test 1: Hash command with default cost
echo "Test 1: Hash with default cost"
if $EXE hash testpassword > /dev/null 2>&1; then
    echo "[PASS] Hash command works"
    ((PASSED++))
else
    echo "[FAIL] Hash command failed"
    ((FAILED++))
fi

# Test 2: Hash with custom cost
echo "Test 2: Hash with custom cost"
if $EXE hash testpassword 12 > /dev/null 2>&1; then
    echo "[PASS] Hash with cost=12 works"
    ((PASSED++))
else
    echo "[FAIL] Hash with cost failed"
    ((FAILED++))
fi

# Test 3: Hash with salt
echo "Test 3: Hash with salt"
if $EXE hash testpassword 10 mysalt > /dev/null 2>&1; then
    echo "[PASS] Hash with salt works"
    ((PASSED++))
else
    echo "[FAIL] Hash with salt failed"
    ((FAILED++))
fi

# Test 4: File encryption
echo "Test 4: File encryption"
echo "This is a test file for encryption" > test_cli.txt
if $EXE encrypt test_cli.txt mypassword > /dev/null 2>&1; then
    if [ -f test_cli.txt.enc ]; then
        echo "[PASS] File encryption works"
        ((PASSED++))
    else
        echo "[FAIL] Encrypted file not created"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption command failed"
    ((FAILED++))
fi

# Test 5: File decryption
echo "Test 5: File decryption"
if $EXE decrypt test_cli.txt.enc mypassword test_cli_dec.txt > /dev/null 2>&1; then
    if [ -f test_cli_dec.txt ]; then
        echo "[PASS] File decryption works"
        ((PASSED++))
    else
        echo "[FAIL] Decrypted file not created"
        ((FAILED++))
    fi
else
    echo "[FAIL] Decryption command failed"
    ((FAILED++))
fi

# Test 6: Content integrity
echo "Test 6: Content integrity"
if cmp -s test_cli.txt test_cli_dec.txt; then
    echo "[PASS] Decrypted content matches original"
    ((PASSED++))
else
    echo "[FAIL] Content mismatch"
    ((FAILED++))
fi

# Test 7: Wrong password rejection
echo "Test 7: Wrong password rejection"
if ! $EXE decrypt test_cli.txt.enc wrongpassword test_wrong.txt > /dev/null 2>&1; then
    echo "[PASS] Wrong password rejected"
    ((PASSED++))
else
    echo "[FAIL] Wrong password not rejected"
    ((FAILED++))
fi

# Test 8: Custom output file
echo "Test 8: Custom output file"
if $EXE encrypt test_cli.txt mypassword custom_output.enc > /dev/null 2>&1; then
    if [ -f custom_output.enc ]; then
        echo "[PASS] Custom output file works"
        ((PASSED++))
    else
        echo "[FAIL] Custom output not created"
        ((FAILED++))
    fi
else
    echo "[FAIL] Custom output failed"
    ((FAILED++))
fi

# Test 9: Decrypt custom output
echo "Test 9: Decrypt custom output"
if $EXE decrypt custom_output.enc mypassword custom_dec.txt > /dev/null 2>&1; then
    if cmp -s test_cli.txt custom_dec.txt; then
        echo "[PASS] Custom output decryption works"
        ((PASSED++))
    else
        echo "[FAIL] Custom output content mismatch"
        ((FAILED++))
    fi
else
    echo "[FAIL] Custom output decryption failed"
    ((FAILED++))
fi

# Test 10: Binary file
echo "Test 10: Binary file encryption"
cp $EXE test_binary.exe
if $EXE encrypt test_binary.exe binpass test_binary.enc > /dev/null 2>&1; then
    if $EXE decrypt test_binary.enc binpass test_binary_dec.exe > /dev/null 2>&1; then
        if cmp -s test_binary.exe test_binary_dec.exe; then
            echo "[PASS] Binary file encryption works"
            ((PASSED++))
        else
            echo "[FAIL] Binary content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Binary decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Binary encryption failed"
    ((FAILED++))
fi

# Test 11: Non-existent file
echo "Test 11: Non-existent file handling"
if ! $EXE encrypt nonexistent.txt pass > /dev/null 2>&1; then
    echo "[PASS] Non-existent file handled correctly"
    ((PASSED++))
else
    echo "[FAIL] Should fail on non-existent file"
    ((FAILED++))
fi

# Test 12: Empty file
echo "Test 12: Empty file"
touch test_empty.txt
if $EXE encrypt test_empty.txt pass test_empty.enc > /dev/null 2>&1; then
    if $EXE decrypt test_empty.enc pass test_empty_dec.txt > /dev/null 2>&1; then
        if cmp -s test_empty.txt test_empty_dec.txt; then
            echo "[PASS] Empty file works"
            ((PASSED++))
        else
            echo "[FAIL] Empty file content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Empty file decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Empty file encryption failed"
    ((FAILED++))
fi

# Test 13: Large file (1MB)
echo "Test 13: Large file (1MB)"
dd if=/dev/zero of=test_large.bin bs=1M count=1 > /dev/null 2>&1
if $EXE encrypt test_large.bin largepass test_large.enc > /dev/null 2>&1; then
    if $EXE decrypt test_large.enc largepass test_large_dec.bin > /dev/null 2>&1; then
        if cmp -s test_large.bin test_large_dec.bin; then
            echo "[PASS] Large file works"
            ((PASSED++))
        else
            echo "[FAIL] Large file content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Large file decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Large file encryption failed"
    ((FAILED++))
fi

# Test 14: Special characters in password
echo "Test 14: Special chars in password"
echo "Test content" > test_special_pass.txt
if $EXE encrypt test_special_pass.txt 'p@ss!w0rd#123' test_special_pass.enc > /dev/null 2>&1; then
    if $EXE decrypt test_special_pass.enc 'p@ss!w0rd#123' test_special_pass_dec.txt > /dev/null 2>&1; then
        if cmp -s test_special_pass.txt test_special_pass_dec.txt; then
            echo "[PASS] Special chars in password works"
            ((PASSED++))
        else
            echo "[FAIL] Special password content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Special password decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Special password encryption failed"
    ((FAILED++))
fi

# Test 15: Multiple files
echo "Test 15: Multiple files"
echo "File 1" > test_multi1.txt
echo "File 2" > test_multi2.txt
$EXE encrypt test_multi1.txt pass1 > /dev/null 2>&1
$EXE encrypt test_multi2.txt pass2 > /dev/null 2>&1
if [ -f test_multi1.txt.enc ] && [ -f test_multi2.txt.enc ]; then
    echo "[PASS] Multiple files encryption works"
    ((PASSED++))
else
    echo "[FAIL] Multiple files encryption failed"
    ((FAILED++))
fi

# Test 16: Hash determinism with salt
echo "Test 16: Hash determinism with salt"
hash1=$($EXE hash testpass 10 salt123 2>/dev/null)
hash2=$($EXE hash testpass 10 salt123 2>/dev/null)
if [ "$hash1" = "$hash2" ] && [ -n "$hash1" ]; then
    echo "[PASS] Hash determinism works"
    ((PASSED++))
else
    echo "[FAIL] Hash not deterministic"
    ((FAILED++))
fi

# Test 17: Different salts produce different hashes
echo "Test 17: Different salts"
hash1=$($EXE hash testpass 10 salt1 2>/dev/null)
hash2=$($EXE hash testpass 10 salt2 2>/dev/null)
if [ "$hash1" != "$hash2" ] && [ -n "$hash1" ] && [ -n "$hash2" ]; then
    echo "[PASS] Different salts work"
    ((PASSED++))
else
    echo "[FAIL] Salts not working"
    ((FAILED++))
fi

# Test 18: Additional encryption test
echo "Test 18: Additional encryption test"
echo "Content" > test_add.txt
if $EXE encrypt test_add.txt pass test_add.enc > /dev/null 2>&1; then
    if $EXE decrypt test_add.enc pass test_add_dec.txt > /dev/null 2>&1; then
        if cmp -s test_add.txt test_add_dec.txt; then
            echo "[PASS] Additional test works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 19: Very long password
echo "Test 19: Very long password"
echo "Test" > test_longpass.txt
longpass="ThisIsAVeryLongPasswordWithMoreThan50CharactersToTestPasswordHandling123456789"
if $EXE encrypt test_longpass.txt "$longpass" > /dev/null 2>&1; then
    if $EXE decrypt test_longpass.txt.enc "$longpass" test_longpass_dec.txt > /dev/null 2>&1; then
        if cmp -s test_longpass.txt test_longpass_dec.txt; then
            echo "[PASS] Long password works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 20: Single character password
echo "Test 20: Single char password"
echo "Test" > test_shortpass.txt
if $EXE encrypt test_shortpass.txt "a" > /dev/null 2>&1; then
    if $EXE decrypt test_shortpass.txt.enc "a" test_shortpass_dec.txt > /dev/null 2>&1; then
        if cmp -s test_shortpass.txt test_shortpass_dec.txt; then
            echo "[PASS] Single char password works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 21: File with only newlines
echo "Test 21: Only newlines"
printf "\n\n\n\n\n" > test_newlines.txt
if $EXE encrypt test_newlines.txt pass > /dev/null 2>&1; then
    if $EXE decrypt test_newlines.txt.enc pass test_newlines_dec.txt > /dev/null 2>&1; then
        if cmp -s test_newlines.txt test_newlines_dec.txt; then
            echo "[PASS] Newlines only works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 22: 10KB file
echo "Test 22: 10KB file"
dd if=/dev/urandom of=test_10kb.bin bs=10K count=1 > /dev/null 2>&1
if $EXE encrypt test_10kb.bin pass > /dev/null 2>&1; then
    if $EXE decrypt test_10kb.bin.enc pass test_10kb_dec.bin > /dev/null 2>&1; then
        if cmp -s test_10kb.bin test_10kb_dec.bin; then
            echo "[PASS] 10KB file works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 23: 100KB file
echo "Test 23: 100KB file"
dd if=/dev/urandom of=test_100kb.bin bs=100K count=1 > /dev/null 2>&1
if $EXE encrypt test_100kb.bin pass > /dev/null 2>&1; then
    if $EXE decrypt test_100kb.bin.enc pass test_100kb_dec.bin > /dev/null 2>&1; then
        if cmp -s test_100kb.bin test_100kb_dec.bin; then
            echo "[PASS] 100KB file works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 24: 5MB file (boundary)
echo "Test 24: 5MB file (boundary)"
dd if=/dev/urandom of=test_5mb.bin bs=5M count=1 > /dev/null 2>&1
if $EXE encrypt test_5mb.bin pass > /dev/null 2>&1; then
    if $EXE decrypt test_5mb.bin.enc pass test_5mb_dec.bin > /dev/null 2>&1; then
        if cmp -s test_5mb.bin test_5mb_dec.bin; then
            echo "[PASS] 5MB file works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 25: Cost factor 8
echo "Test 25: Hash cost=8"
if $EXE hash testpass 8 > /dev/null 2>&1; then
    echo "[PASS] Cost=8 works"
    ((PASSED++))
else
    echo "[FAIL] Cost=8 failed"
    ((FAILED++))
fi

# Test 26: Cost factor 14
echo "Test 26: Hash cost=14"
if $EXE hash testpass 14 > /dev/null 2>&1; then
    echo "[PASS] Cost=14 works"
    ((PASSED++))
else
    echo "[FAIL] Cost=14 failed"
    ((FAILED++))
fi

# Test 27: Numeric password
echo "Test 27: Numeric password"
echo "Test" > test_numpass.txt
if $EXE encrypt test_numpass.txt "123456789" > /dev/null 2>&1; then
    if $EXE decrypt test_numpass.txt.enc "123456789" test_numpass_dec.txt > /dev/null 2>&1; then
        if cmp -s test_numpass.txt test_numpass_dec.txt; then
            echo "[PASS] Numeric password works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 28: Password with spaces
echo "Test 28: Password with spaces"
echo "Test" > test_spacepass.txt
if $EXE encrypt test_spacepass.txt "pass word" > /dev/null 2>&1; then
    if $EXE decrypt test_spacepass.txt.enc "pass word" test_spacepass_dec.txt > /dev/null 2>&1; then
        if cmp -s test_spacepass.txt test_spacepass_dec.txt; then
            echo "[PASS] Password with spaces works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Test 29: Round trip encryption
echo "Test 29: Round trip"
echo "Round trip test" > test_rt.txt
$EXE encrypt test_rt.txt pass test_rt1.enc > /dev/null 2>&1
$EXE decrypt test_rt1.enc pass test_rt1_dec.txt > /dev/null 2>&1
$EXE encrypt test_rt1_dec.txt pass test_rt2.enc > /dev/null 2>&1
$EXE decrypt test_rt2.enc pass test_rt2_dec.txt > /dev/null 2>&1
if cmp -s test_rt.txt test_rt2_dec.txt; then
    echo "[PASS] Round trip works"
    ((PASSED++))
else
    echo "[FAIL] Round trip failed"
    ((FAILED++))
fi

# Test 30: Text file with UTF-8
echo "Test 30: UTF-8 content"
echo "Hello 世界 🔐" > test_utf8.txt
if $EXE encrypt test_utf8.txt pass > /dev/null 2>&1; then
    if $EXE decrypt test_utf8.txt.enc pass test_utf8_dec.txt > /dev/null 2>&1; then
        if cmp -s test_utf8.txt test_utf8_dec.txt; then
            echo "[PASS] UTF-8 works"
            ((PASSED++))
        else
            echo "[FAIL] Content mismatch"
            ((FAILED++))
        fi
    else
        echo "[FAIL] Decryption failed"
        ((FAILED++))
    fi
else
    echo "[FAIL] Encryption failed"
    ((FAILED++))
fi

# Cleanup
echo
echo "Cleaning up test files..."
rm -f test_cli.txt test_cli.txt.enc test_cli_dec.txt test_wrong.txt
rm -f custom_output.enc custom_dec.txt
rm -f test_binary.exe test_binary.enc test_binary_dec.exe
rm -f test_empty.txt test_empty.enc test_empty_dec.txt
rm -f test_large.bin test_large.enc test_large_dec.bin
rm -f test_special_pass.txt test_special_pass.enc test_special_pass_dec.txt
rm -f test_multi1.txt test_multi1.txt.enc test_multi2.txt test_multi2.txt.enc
rm -f test_add.txt test_add.enc test_add_dec.txt
rm -f "test file.txt" "test file.txt.enc" "test file dec.txt"
rm -f test_longpass.txt test_longpass.txt.enc test_longpass_dec.txt
rm -f test_shortpass.txt test_shortpass.txt.enc test_shortpass_dec.txt
rm -f test_newlines.txt test_newlines.txt.enc test_newlines_dec.txt
rm -f test_10kb.bin test_10kb.bin.enc test_10kb_dec.bin
rm -f test_100kb.bin test_100kb.bin.enc test_100kb_dec.bin
rm -f test_5mb.bin test_5mb.bin.enc test_5mb_dec.bin
rm -f test_numpass.txt test_numpass.txt.enc test_numpass_dec.txt
rm -f test_spacepass.txt test_spacepass.txt.enc test_spacepass_dec.txt
rm -f test_rt.txt test_rt1.enc test_rt1_dec.txt test_rt2.enc test_rt2_dec.txt
rm -f test_utf8.txt test_utf8.txt.enc test_utf8_dec.txt

echo
echo "========================================"
echo "Test Results"
echo "========================================"
echo "Total Tests: 30"
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "========================================"

if [ $FAILED -eq 0 ]; then
    echo
    echo "All tests passed! ✓"
    exit 0
else
    echo
    echo "Some tests failed! ✗"
    exit 1
fi
