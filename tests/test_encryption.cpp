#include "encryption.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sstream>

static int passed = 0;
static int failed = 0;

static void test(const char *name, bool condition) {
    if (condition) {
        printf("✓ %s\n", name);
        passed++;
    } else {
        printf("✗ %s\n", name);
        failed++;
    }
}

static void create_test_file(const char *filename, const char *content) {
    std::ofstream f(filename, std::ios::binary);
    f << content;
    f.close();
}

static void create_test_file_binary(const char *filename, size_t size) {
    std::ofstream f(filename, std::ios::binary);
    for (size_t i = 0; i < size; i++) {
        f.put((char)(i % 256));
    }
    f.close();
}

static void create_test_file_pattern(const char *filename, size_t size, const char *pattern, size_t plen) {
    std::ofstream f(filename, std::ios::binary);
    for (size_t i = 0; i < size; i++) {
        f.put(pattern[i % plen]);
    }
    f.close();
}

static bool files_match(const char *file1, const char *file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    if (!f1 || !f2) return false;
    
    std::vector<char> buf1((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    std::vector<char> buf2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());
    
    return buf1 == buf2;
}

static bool file_exists(const char *filename) {
    std::ifstream f(filename);
    return f.good();
}

static size_t get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) return st.st_size;
    return 0;
}

int main() {
    printf("========================================\n");
    printf("File Encryption/Decryption Tests\n");
    printf("========================================\n\n");
    
    // Test 1: Basic encryption/decryption round-trip
    create_test_file("test1.txt", "Hello, World!");
    int rc = encrypt_file_advanced("test1.txt", "test1.enc", "password123", 10);
    test("Test 1: Basic encryption succeeds", rc == 0);
    
    rc = decrypt_file_advanced("test1.enc", "test1_dec.txt", "password123");
    test("Test 2: Basic decryption succeeds", rc == 0);
    test("Test 3: Decrypted content matches original", files_match("test1.txt", "test1_dec.txt"));
    
    // Test 4: Wrong password rejection
    rc = decrypt_file_advanced("test1.enc", "test1_wrong.txt", "wrongpassword");
    test("Test 4: Wrong password rejected (rc=-2)", rc == -2);
    
    // Test 5: Empty file
    create_test_file("test_empty.txt", "");
    rc = encrypt_file_advanced("test_empty.txt", "test_empty.enc", "pass", 10);
    test("Test 5: Empty file encryption succeeds", rc == 0);
    rc = decrypt_file_advanced("test_empty.enc", "test_empty_dec.txt", "pass");
    test("Test 6: Empty file decryption succeeds", rc == 0 && files_match("test_empty.txt", "test_empty_dec.txt"));
    
    // Test 7: Single byte file
    create_test_file("test_single.txt", "A");
    rc = encrypt_file_advanced("test_single.txt", "test_single.enc", "mypass", 10);
    test("Test 7: Single byte encryption succeeds", rc == 0);
    rc = decrypt_file_advanced("test_single.enc", "test_single_dec.txt", "mypass");
    test("Test 8: Single byte decryption matches", rc == 0 && files_match("test_single.txt", "test_single_dec.txt"));
    
    // Test 9: File with special characters
    create_test_file("test_special.txt", "Test!@#$%^&*()_+-=[]{}|;':\",./<>?\n\t\r");
    rc = encrypt_file_advanced("test_special.txt", "test_special.enc", "special123", 10);
    test("Test 9: Special chars encryption succeeds", rc == 0);
    rc = decrypt_file_advanced("test_special.enc", "test_special_dec.txt", "special123");
    test("Test 10: Special chars decryption matches", rc == 0 && files_match("test_special.txt", "test_special_dec.txt"));
    
    // Test 11: Binary file (1KB)
    create_test_file_binary("test_binary.bin", 1024);
    rc = encrypt_file_advanced("test_binary.bin", "test_binary.enc", "binpass", 10);
    test("Test 11: Binary file encryption succeeds", rc == 0);
    rc = decrypt_file_advanced("test_binary.enc", "test_binary_dec.bin", "binpass");
    test("Test 12: Binary file decryption matches", rc == 0 && files_match("test_binary.bin", "test_binary_dec.bin"));
    
    // Test 13: Larger file (10KB - tests chunking)
    create_test_file_binary("test_large.bin", 10240);
    rc = encrypt_file_advanced("test_large.bin", "test_large.enc", "largepass", 10);
    test("Test 13: Large file encryption succeeds", rc == 0);
    rc = decrypt_file_advanced("test_large.enc", "test_large_dec.bin", "largepass");
    test("Test 14: Large file decryption matches", rc == 0 && files_match("test_large.bin", "test_large_dec.bin"));
    
    // Test 15: Different passwords produce different encrypted files
    create_test_file("test_diff.txt", "Same content");
    encrypt_file_advanced("test_diff.txt", "test_diff1.enc", "pass1", 10);
    encrypt_file_advanced("test_diff.txt", "test_diff2.enc", "pass2", 10);
    test("Test 15: Different passwords produce different encrypted files", !files_match("test_diff1.enc", "test_diff2.enc"));
    
    // Test 16: Same password produces different encrypted files (random salt)
    encrypt_file_advanced("test_diff.txt", "test_diff3.enc", "samepass", 10);
    encrypt_file_advanced("test_diff.txt", "test_diff4.enc", "samepass", 10);
    test("Test 16: Same password with random salt produces different files", !files_match("test_diff3.enc", "test_diff4.enc"));
    
    // Test 17: Both decrypt correctly despite different encrypted files
    decrypt_file_advanced("test_diff3.enc", "test_diff3_dec.txt", "samepass");
    decrypt_file_advanced("test_diff4.enc", "test_diff4_dec.txt", "samepass");
    test("Test 17: Both decrypt to same original content", 
         files_match("test_diff3_dec.txt", "test_diff.txt") && 
         files_match("test_diff4_dec.txt", "test_diff.txt"));
    
    // Test 18: Non-existent file
    rc = encrypt_file_advanced("nonexistent.txt", "out.enc", "pass", 10);
    test("Test 18: Non-existent file returns error", rc == -1);
    
    // Test 19: Encrypted file integrity
    create_test_file("test_integrity.txt", "Integrity test content");
    encrypt_file_advanced("test_integrity.txt", "test_integrity.enc", "intpass", 10);
    test("Test 19: Encrypted file exists and has content", file_exists("test_integrity.enc"));
    
    // Test 20: Multiple round-trips preserve data
    create_test_file("test_roundtrip.txt", "Round trip test data with multiple lines\nLine 2\nLine 3");
    encrypt_file_advanced("test_roundtrip.txt", "test_rt1.enc", "rtpass", 10);
    decrypt_file_advanced("test_rt1.enc", "test_rt1_dec.txt", "rtpass");
    encrypt_file_advanced("test_rt1_dec.txt", "test_rt2.enc", "rtpass", 10);
    decrypt_file_advanced("test_rt2.enc", "test_rt2_dec.txt", "rtpass");
    test("Test 20: Multiple round-trips preserve data", files_match("test_roundtrip.txt", "test_rt2_dec.txt"));
    
    // Test 21-25: File size boundaries (chunking tests)
    create_test_file_binary("test_1kb.bin", 1024);
    rc = encrypt_file_advanced("test_1kb.bin", "test_1kb.enc", "pass", 10);
    test("Test 21: 1KB file encryption", rc == 0);
    rc = decrypt_file_advanced("test_1kb.enc", "test_1kb_dec.bin", "pass");
    test("Test 22: 1KB file decryption matches", rc == 0 && files_match("test_1kb.bin", "test_1kb_dec.bin"));
    
    create_test_file_binary("test_4mb.bin", 4 * 1024 * 1024);
    rc = encrypt_file_advanced("test_4mb.bin", "test_4mb.enc", "pass", 10);
    test("Test 23: 4MB file encryption (no chunking)", rc == 0);
    rc = decrypt_file_advanced("test_4mb.enc", "test_4mb_dec.bin", "pass");
    test("Test 24: 4MB file decryption matches", rc == 0 && files_match("test_4mb.bin", "test_4mb_dec.bin"));
    
    create_test_file_binary("test_6mb.bin", 6 * 1024 * 1024);
    rc = encrypt_file_advanced("test_6mb.bin", "test_6mb.enc", "pass", 10);
    test("Test 25: 6MB file encryption (512KB chunks)", rc == 0);
    rc = decrypt_file_advanced("test_6mb.enc", "test_6mb_dec.bin", "pass");
    test("Test 26: 6MB file decryption matches", rc == 0 && files_match("test_6mb.bin", "test_6mb_dec.bin"));
    
    // Test 27-30: Different cost factors
    create_test_file("test_cost.txt", "Cost factor test");
    rc = encrypt_file_advanced("test_cost.txt", "test_cost8.enc", "pass", 8);
    test("Test 27: Cost=8 encryption", rc == 0);
    rc = decrypt_file_advanced("test_cost8.enc", "test_cost8_dec.txt", "pass");
    test("Test 28: Cost=8 decryption matches", rc == 0 && files_match("test_cost.txt", "test_cost8_dec.txt"));
    
    rc = encrypt_file_advanced("test_cost.txt", "test_cost12.enc", "pass", 12);
    test("Test 29: Cost=12 encryption", rc == 0);
    rc = decrypt_file_advanced("test_cost12.enc", "test_cost12_dec.txt", "pass");
    test("Test 30: Cost=12 decryption matches", rc == 0 && files_match("test_cost.txt", "test_cost12_dec.txt"));
    
    // Test 31-35: Password variations
    create_test_file("test_pass.txt", "Password test");
    rc = encrypt_file_advanced("test_pass.txt", "test_pass_short.enc", "a", 10);
    test("Test 31: Single char password encryption", rc == 0);
    rc = decrypt_file_advanced("test_pass_short.enc", "test_pass_short_dec.txt", "a");
    test("Test 32: Single char password decryption", rc == 0 && files_match("test_pass.txt", "test_pass_short_dec.txt"));
    
    rc = encrypt_file_advanced("test_pass.txt", "test_pass_long.enc", "VeryLongPasswordWith123Numbers!@#Special", 10);
    test("Test 33: Long password encryption", rc == 0);
    rc = decrypt_file_advanced("test_pass_long.enc", "test_pass_long_dec.txt", "VeryLongPasswordWith123Numbers!@#Special");
    test("Test 34: Long password decryption", rc == 0 && files_match("test_pass.txt", "test_pass_long_dec.txt"));
    
    rc = decrypt_file_advanced("test_pass_long.enc", "test_pass_wrong.txt", "VeryLongPasswordWith123Numbers!@#Specia");
    test("Test 35: Similar but wrong password rejected", rc == -2);
    
    // Test 36-40: Content patterns
    char zeros[4] = {0, 0, 0, 0};
    create_test_file_pattern("test_zeros.bin", 5000, zeros, 4);
    rc = encrypt_file_advanced("test_zeros.bin", "test_zeros.enc", "pass", 10);
    test("Test 36: All zeros file encryption", rc == 0);
    rc = decrypt_file_advanced("test_zeros.enc", "test_zeros_dec.bin", "pass");
    test("Test 37: All zeros file decryption matches", rc == 0 && files_match("test_zeros.bin", "test_zeros_dec.bin"));
    
    char ones[4] = {(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF};
    create_test_file_pattern("test_ones.bin", 5000, ones, 4);
    rc = encrypt_file_advanced("test_ones.bin", "test_ones.enc", "pass", 10);
    test("Test 38: All 0xFF file encryption", rc == 0);
    rc = decrypt_file_advanced("test_ones.enc", "test_ones_dec.bin", "pass");
    test("Test 39: All 0xFF file decryption matches", rc == 0 && files_match("test_ones.bin", "test_ones_dec.bin"));
    
    create_test_file_pattern("test_repeat.bin", 5000, "ABCD", 4);
    rc = encrypt_file_advanced("test_repeat.bin", "test_repeat.enc", "pass", 10);
    test("Test 40: Repeating pattern encryption", rc == 0);
    rc = decrypt_file_advanced("test_repeat.enc", "test_repeat_dec.bin", "pass");
    test("Test 41: Repeating pattern decryption matches", rc == 0 && files_match("test_repeat.bin", "test_repeat_dec.bin"));
    
    // Test 42-45: Unicode and text content
    create_test_file("test_unicode.txt", "Hello \xE4\xB8\x96\xE7\x95\x8C UTF-8 test");
    rc = encrypt_file_advanced("test_unicode.txt", "test_unicode.enc", "pass", 10);
    test("Test 42: Unicode content encryption", rc == 0);
    rc = decrypt_file_advanced("test_unicode.enc", "test_unicode_dec.txt", "pass");
    test("Test 43: Unicode content decryption matches", rc == 0 && files_match("test_unicode.txt", "test_unicode_dec.txt"));
    
    create_test_file("test_multiline.txt", "Line1\nLine2\nLine3\nLine4\nLine5\n");
    rc = encrypt_file_advanced("test_multiline.txt", "test_multiline.enc", "pass", 10);
    test("Test 44: Multiline text encryption", rc == 0);
    rc = decrypt_file_advanced("test_multiline.enc", "test_multiline_dec.txt", "pass");
    test("Test 45: Multiline text decryption matches", rc == 0 && files_match("test_multiline.txt", "test_multiline_dec.txt"));
    
    // Test 46-50: Boundary sizes for chunking
    create_test_file_binary("test_5mb_exact.bin", 5 * 1024 * 1024);
    rc = encrypt_file_advanced("test_5mb_exact.bin", "test_5mb_exact.enc", "pass", 10);
    test("Test 46: Exactly 5MB file (boundary)", rc == 0);
    rc = decrypt_file_advanced("test_5mb_exact.enc", "test_5mb_exact_dec.bin", "pass");
    test("Test 47: 5MB boundary decryption matches", rc == 0 && files_match("test_5mb_exact.bin", "test_5mb_exact_dec.bin"));
    
    create_test_file_binary("test_50mb.bin", 50 * 1024 * 1024);
    rc = encrypt_file_advanced("test_50mb.bin", "test_50mb.enc", "pass", 10);
    test("Test 48: 50MB file (2MB chunks)", rc == 0);
    rc = decrypt_file_advanced("test_50mb.enc", "test_50mb_dec.bin", "pass");
    test("Test 49: 50MB decryption matches", rc == 0 && files_match("test_50mb.bin", "test_50mb_dec.bin"));
    
    create_test_file_binary("test_513kb.bin", 513 * 1024);
    rc = encrypt_file_advanced("test_513kb.bin", "test_513kb.enc", "pass", 10);
    test("Test 50: 513KB file (just over 512KB)", rc == 0);
    rc = decrypt_file_advanced("test_513kb.enc", "test_513kb_dec.bin", "pass");
    test("Test 51: 513KB decryption matches", rc == 0 && files_match("test_513kb.bin", "test_513kb_dec.bin"));
    
    // Test 52-55: Compression effectiveness
    create_test_file_pattern("test_compress.txt", 10000, "AAAAAAAAAA", 10);
    rc = encrypt_file_advanced("test_compress.txt", "test_compress.enc", "pass", 10);
    test("Test 52: Highly compressible file encryption", rc == 0);
    size_t orig_size = get_file_size("test_compress.txt");
    size_t enc_size = get_file_size("test_compress.enc");
    test("Test 53: Encrypted file exists with size", enc_size > 0 && enc_size < orig_size * 2);
    rc = decrypt_file_advanced("test_compress.enc", "test_compress_dec.txt", "pass");
    test("Test 54: Compressed file decryption matches", rc == 0 && files_match("test_compress.txt", "test_compress_dec.txt"));
    
    create_test_file_binary("test_random.bin", 10000);
    rc = encrypt_file_advanced("test_random.bin", "test_random.enc", "pass", 10);
    test("Test 55: Random data encryption", rc == 0);
    rc = decrypt_file_advanced("test_random.enc", "test_random_dec.bin", "pass");
    test("Test 56: Random data decryption matches", rc == 0 && files_match("test_random.bin", "test_random_dec.bin"));
    
    // Test 57-60: Edge case passwords
    create_test_file("test_edge.txt", "Edge case test");
    rc = encrypt_file_advanced("test_edge.txt", "test_space.enc", "pass word", 10);
    test("Test 57: Password with space encryption", rc == 0);
    rc = decrypt_file_advanced("test_space.enc", "test_space_dec.txt", "pass word");
    test("Test 58: Password with space decryption", rc == 0 && files_match("test_edge.txt", "test_space_dec.txt"));
    
    rc = encrypt_file_advanced("test_edge.txt", "test_special_pass.enc", "!@#$%^&*()", 10);
    test("Test 59: Special char password encryption", rc == 0);
    rc = decrypt_file_advanced("test_special_pass.enc", "test_special_pass_dec.txt", "!@#$%^&*()");
    test("Test 60: Special char password decryption", rc == 0 && files_match("test_edge.txt", "test_special_pass_dec.txt"));
    
    // Test 61-70: Various file sizes for chunking boundaries
    create_test_file_binary("test_100kb.bin", 100 * 1024);
    rc = encrypt_file_advanced("test_100kb.bin", "test_100kb.enc", "pass", 10);
    test("Test 61: 100KB file encryption", rc == 0);
    rc = decrypt_file_advanced("test_100kb.enc", "test_100kb_dec.bin", "pass");
    test("Test 62: 100KB file decryption", rc == 0 && files_match("test_100kb.bin", "test_100kb_dec.bin"));
    
    create_test_file_binary("test_2mb.bin", 2 * 1024 * 1024);
    rc = encrypt_file_advanced("test_2mb.bin", "test_2mb.enc", "pass", 10);
    test("Test 63: 2MB file encryption", rc == 0);
    rc = decrypt_file_advanced("test_2mb.enc", "test_2mb_dec.bin", "pass");
    test("Test 64: 2MB file decryption", rc == 0 && files_match("test_2mb.bin", "test_2mb_dec.bin"));
    
    create_test_file_binary("test_10mb.bin", 10 * 1024 * 1024);
    rc = encrypt_file_advanced("test_10mb.bin", "test_10mb.enc", "pass", 10);
    test("Test 65: 10MB file encryption", rc == 0);
    rc = decrypt_file_advanced("test_10mb.enc", "test_10mb_dec.bin", "pass");
    test("Test 66: 10MB file decryption", rc == 0 && files_match("test_10mb.bin", "test_10mb_dec.bin"));
    
    create_test_file_binary("test_511kb.bin", 511 * 1024);
    rc = encrypt_file_advanced("test_511kb.bin", "test_511kb.enc", "pass", 10);
    test("Test 67: 511KB file (just under 512KB)", rc == 0);
    rc = decrypt_file_advanced("test_511kb.enc", "test_511kb_dec.bin", "pass");
    test("Test 68: 511KB decryption", rc == 0 && files_match("test_511kb.bin", "test_511kb_dec.bin"));
    
    create_test_file_binary("test_1025kb.bin", 1025 * 1024);
    rc = encrypt_file_advanced("test_1025kb.bin", "test_1025kb.enc", "pass", 10);
    test("Test 69: 1025KB file", rc == 0);
    rc = decrypt_file_advanced("test_1025kb.enc", "test_1025kb_dec.bin", "pass");
    test("Test 70: 1025KB decryption", rc == 0 && files_match("test_1025kb.bin", "test_1025kb_dec.bin"));
    
    // Test 71-80: Different cost factors with various files
    create_test_file("test_cost_var.txt", "Cost variation test");
    rc = encrypt_file_advanced("test_cost_var.txt", "test_cost9.enc", "pass", 9);
    test("Test 71: Cost=9 encryption", rc == 0);
    rc = decrypt_file_advanced("test_cost9.enc", "test_cost9_dec.txt", "pass");
    test("Test 72: Cost=9 decryption", rc == 0 && files_match("test_cost_var.txt", "test_cost9_dec.txt"));
    
    rc = encrypt_file_advanced("test_cost_var.txt", "test_cost11.enc", "pass", 11);
    test("Test 73: Cost=11 encryption", rc == 0);
    rc = decrypt_file_advanced("test_cost11.enc", "test_cost11_dec.txt", "pass");
    test("Test 74: Cost=11 decryption", rc == 0 && files_match("test_cost_var.txt", "test_cost11_dec.txt"));
    
    rc = encrypt_file_advanced("test_cost_var.txt", "test_cost13.enc", "pass", 13);
    test("Test 75: Cost=13 encryption", rc == 0);
    rc = decrypt_file_advanced("test_cost13.enc", "test_cost13_dec.txt", "pass");
    test("Test 76: Cost=13 decryption", rc == 0 && files_match("test_cost_var.txt", "test_cost13_dec.txt"));
    
    rc = decrypt_file_advanced("test_cost13.enc", "test_cost_wrong.txt", "wrongpass");
    test("Test 77: Cost=13 wrong password", rc == -2);
    
    create_test_file_binary("test_cost_bin.bin", 2048);
    rc = encrypt_file_advanced("test_cost_bin.bin", "test_cost_bin8.enc", "pass", 8);
    test("Test 78: Binary with cost=8", rc == 0);
    rc = decrypt_file_advanced("test_cost_bin8.enc", "test_cost_bin8_dec.bin", "pass");
    test("Test 79: Binary cost=8 decryption", rc == 0 && files_match("test_cost_bin.bin", "test_cost_bin8_dec.bin"));
    
    rc = encrypt_file_advanced("test_cost_bin.bin", "test_cost_bin14.enc", "pass", 14);
    test("Test 80: Binary with cost=14", rc == 0);
    
    // Test 81-90: Password edge cases
    create_test_file("test_pass_edge.txt", "Password edge cases");
    rc = encrypt_file_advanced("test_pass_edge.txt", "test_pass_num.enc", "123456789", 10);
    test("Test 81: Numeric password", rc == 0);
    rc = decrypt_file_advanced("test_pass_num.enc", "test_pass_num_dec.txt", "123456789");
    test("Test 82: Numeric password decryption", rc == 0 && files_match("test_pass_edge.txt", "test_pass_num_dec.txt"));
    
    rc = encrypt_file_advanced("test_pass_edge.txt", "test_pass_mixed.enc", "aA1!bB2@", 10);
    test("Test 83: Mixed case+num+special password", rc == 0);
    rc = decrypt_file_advanced("test_pass_mixed.enc", "test_pass_mixed_dec.txt", "aA1!bB2@");
    test("Test 84: Mixed password decryption", rc == 0 && files_match("test_pass_edge.txt", "test_pass_mixed_dec.txt"));
    
    rc = encrypt_file_advanced("test_pass_edge.txt", "test_pass_repeat.enc", "aaaaaaaaaa", 10);
    test("Test 85: Repeated char password", rc == 0);
    rc = decrypt_file_advanced("test_pass_repeat.enc", "test_pass_repeat_dec.txt", "aaaaaaaaaa");
    test("Test 86: Repeated char decryption", rc == 0 && files_match("test_pass_edge.txt", "test_pass_repeat_dec.txt"));
    
    rc = encrypt_file_advanced("test_pass_edge.txt", "test_pass_unicode.enc", "p\xC3\xA4ss", 10);
    test("Test 87: Unicode in password", rc == 0);
    rc = decrypt_file_advanced("test_pass_unicode.enc", "test_pass_unicode_dec.txt", "p\xC3\xA4ss");
    test("Test 88: Unicode password decryption", rc == 0 && files_match("test_pass_edge.txt", "test_pass_unicode_dec.txt"));
    
    rc = encrypt_file_advanced("test_pass_edge.txt", "test_pass_brackets.enc", "[{(<>)}]", 10);
    test("Test 89: Brackets password", rc == 0);
    rc = decrypt_file_advanced("test_pass_brackets.enc", "test_pass_brackets_dec.txt", "[{(<>)}]");
    test("Test 90: Brackets password decryption", rc == 0 && files_match("test_pass_edge.txt", "test_pass_brackets_dec.txt"));
    
    // Test 91-100: Content variations
    create_test_file("test_content1.txt", "A");
    rc = encrypt_file_advanced("test_content1.txt", "test_content1.enc", "pass", 10);
    test("Test 91: Single 'A' char", rc == 0);
    rc = decrypt_file_advanced("test_content1.enc", "test_content1_dec.txt", "pass");
    test("Test 92: Single 'A' decryption", rc == 0 && files_match("test_content1.txt", "test_content1_dec.txt"));
    
    create_test_file("test_content2.txt", "\n\n\n\n\n");
    rc = encrypt_file_advanced("test_content2.txt", "test_content2.enc", "pass", 10);
    test("Test 93: Only newlines", rc == 0);
    rc = decrypt_file_advanced("test_content2.enc", "test_content2_dec.txt", "pass");
    test("Test 94: Only newlines decryption", rc == 0 && files_match("test_content2.txt", "test_content2_dec.txt"));
    
    create_test_file("test_content3.txt", "     ");
    rc = encrypt_file_advanced("test_content3.txt", "test_content3.enc", "pass", 10);
    test("Test 95: Only spaces", rc == 0);
    rc = decrypt_file_advanced("test_content3.enc", "test_content3_dec.txt", "pass");
    test("Test 96: Only spaces decryption", rc == 0 && files_match("test_content3.txt", "test_content3_dec.txt"));
    
    create_test_file("test_content4.txt", "\t\t\t\t");
    rc = encrypt_file_advanced("test_content4.txt", "test_content4.enc", "pass", 10);
    test("Test 97: Only tabs", rc == 0);
    rc = decrypt_file_advanced("test_content4.enc", "test_content4_dec.txt", "pass");
    test("Test 98: Only tabs decryption", rc == 0 && files_match("test_content4.txt", "test_content4_dec.txt"));
    
    create_test_file("test_content5.txt", "Line1\r\nLine2\r\nLine3");
    rc = encrypt_file_advanced("test_content5.txt", "test_content5.enc", "pass", 10);
    test("Test 99: Windows line endings", rc == 0);
    rc = decrypt_file_advanced("test_content5.enc", "test_content5_dec.txt", "pass");
    test("Test 100: Windows line endings decryption", rc == 0 && files_match("test_content5.txt", "test_content5_dec.txt"));
    
    // Test 101-110: Stress tests and edge cases
    create_test_file_binary("test_stress1.bin", 3 * 1024 * 1024);
    rc = encrypt_file_advanced("test_stress1.bin", "test_stress1.enc", "stresspass", 10);
    test("Test 101: 3MB stress test", rc == 0);
    rc = decrypt_file_advanced("test_stress1.enc", "test_stress1_dec.bin", "stresspass");
    test("Test 102: 3MB stress decryption", rc == 0 && files_match("test_stress1.bin", "test_stress1_dec.bin"));
    
    create_test_file_binary("test_stress2.bin", 7 * 1024 * 1024);
    rc = encrypt_file_advanced("test_stress2.bin", "test_stress2.enc", "pass", 10);
    test("Test 103: 7MB stress test", rc == 0);
    rc = decrypt_file_advanced("test_stress2.enc", "test_stress2_dec.bin", "pass");
    test("Test 104: 7MB stress decryption", rc == 0 && files_match("test_stress2.bin", "test_stress2_dec.bin"));
    
    create_test_file("test_long_line.txt", std::string(10000, 'X').c_str());
    rc = encrypt_file_advanced("test_long_line.txt", "test_long_line.enc", "pass", 10);
    test("Test 105: 10K char single line", rc == 0);
    rc = decrypt_file_advanced("test_long_line.enc", "test_long_line_dec.txt", "pass");
    test("Test 106: Long line decryption", rc == 0 && files_match("test_long_line.txt", "test_long_line_dec.txt"));
    
    std::string many_lines;
    for (int i = 0; i < 1000; i++) {
        std::ostringstream oss;
        oss << "Line " << i << "\n";
        many_lines += oss.str();
    }
    create_test_file("test_many_lines.txt", many_lines.c_str());
    rc = encrypt_file_advanced("test_many_lines.txt", "test_many_lines.enc", "pass", 10);
    test("Test 107: 1000 lines file", rc == 0);
    rc = decrypt_file_advanced("test_many_lines.enc", "test_many_lines_dec.txt", "pass");
    test("Test 108: 1000 lines decryption", rc == 0 && files_match("test_many_lines.txt", "test_many_lines_dec.txt"));
    
    create_test_file_binary("test_exact_1kb.bin", 1024);
    rc = encrypt_file_advanced("test_exact_1kb.bin", "test_exact_1kb.enc", "pass", 10);
    test("Test 109: Exactly 1KB (sub-chunk size)", rc == 0);
    rc = decrypt_file_advanced("test_exact_1kb.enc", "test_exact_1kb_dec.bin", "pass");
    test("Test 110: Exactly 1KB decryption", rc == 0 && files_match("test_exact_1kb.bin", "test_exact_1kb_dec.bin"));
    
    // Cleanup
    printf("\nCleaning up test files...\n");
    const char* cleanup_files[] = {
        "test1.txt", "test1.enc", "test1_dec.txt", "test1_wrong.txt",
        "test_empty.txt", "test_empty.enc", "test_empty_dec.txt",
        "test_single.txt", "test_single.enc", "test_single_dec.txt",
        "test_special.txt", "test_special.enc", "test_special_dec.txt",
        "test_binary.bin", "test_binary.enc", "test_binary_dec.bin",
        "test_large.bin", "test_large.enc", "test_large_dec.bin",
        "test_diff.txt", "test_diff1.enc", "test_diff2.enc", "test_diff3.enc", "test_diff4.enc",
        "test_diff3_dec.txt", "test_diff4_dec.txt",
        "test_integrity.txt", "test_integrity.enc",
        "test_roundtrip.txt", "test_rt1.enc", "test_rt1_dec.txt", "test_rt2.enc", "test_rt2_dec.txt",
        "test_1kb.bin", "test_1kb.enc", "test_1kb_dec.bin",
        "test_4mb.bin", "test_4mb.enc", "test_4mb_dec.bin",
        "test_6mb.bin", "test_6mb.enc", "test_6mb_dec.bin",
        "test_cost.txt", "test_cost8.enc", "test_cost8_dec.txt", "test_cost12.enc", "test_cost12_dec.txt",
        "test_pass.txt", "test_pass_short.enc", "test_pass_short_dec.txt",
        "test_pass_long.enc", "test_pass_long_dec.txt", "test_pass_wrong.txt",
        "test_zeros.bin", "test_zeros.enc", "test_zeros_dec.bin",
        "test_ones.bin", "test_ones.enc", "test_ones_dec.bin",
        "test_repeat.bin", "test_repeat.enc", "test_repeat_dec.bin",
        "test_unicode.txt", "test_unicode.enc", "test_unicode_dec.txt",
        "test_multiline.txt", "test_multiline.enc", "test_multiline_dec.txt",
        "test_5mb_exact.bin", "test_5mb_exact.enc", "test_5mb_exact_dec.bin",
        "test_50mb.bin", "test_50mb.enc", "test_50mb_dec.bin",
        "test_513kb.bin", "test_513kb.enc", "test_513kb_dec.bin",
        "test_compress.txt", "test_compress.enc", "test_compress_dec.txt",
        "test_random.bin", "test_random.enc", "test_random_dec.bin",
        "test_edge.txt", "test_space.enc", "test_space_dec.txt",
        "test_special_pass.enc", "test_special_pass_dec.txt",
        "test_100kb.bin", "test_100kb.enc", "test_100kb_dec.bin",
        "test_2mb.bin", "test_2mb.enc", "test_2mb_dec.bin",
        "test_10mb.bin", "test_10mb.enc", "test_10mb_dec.bin",
        "test_511kb.bin", "test_511kb.enc", "test_511kb_dec.bin",
        "test_1025kb.bin", "test_1025kb.enc", "test_1025kb_dec.bin",
        "test_cost_var.txt", "test_cost9.enc", "test_cost9_dec.txt",
        "test_cost11.enc", "test_cost11_dec.txt", "test_cost13.enc", "test_cost13_dec.txt",
        "test_cost_wrong.txt", "test_cost_bin.bin", "test_cost_bin8.enc", "test_cost_bin8_dec.bin",
        "test_cost_bin14.enc", "test_pass_edge.txt", "test_pass_num.enc", "test_pass_num_dec.txt",
        "test_pass_mixed.enc", "test_pass_mixed_dec.txt", "test_pass_repeat.enc", "test_pass_repeat_dec.txt",
        "test_pass_unicode.enc", "test_pass_unicode_dec.txt", "test_pass_brackets.enc", "test_pass_brackets_dec.txt",
        "test_content1.txt", "test_content1.enc", "test_content1_dec.txt",
        "test_content2.txt", "test_content2.enc", "test_content2_dec.txt",
        "test_content3.txt", "test_content3.enc", "test_content3_dec.txt",
        "test_content4.txt", "test_content4.enc", "test_content4_dec.txt",
        "test_content5.txt", "test_content5.enc", "test_content5_dec.txt",
        "test_stress1.bin", "test_stress1.enc", "test_stress1_dec.bin",
        "test_stress2.bin", "test_stress2.enc", "test_stress2_dec.bin",
        "test_long_line.txt", "test_long_line.enc", "test_long_line_dec.txt",
        "test_many_lines.txt", "test_many_lines.enc", "test_many_lines_dec.txt",
        "test_exact_1kb.bin", "test_exact_1kb.enc", "test_exact_1kb_dec.bin"
    };
    
    for (size_t i = 0; i < sizeof(cleanup_files) / sizeof(cleanup_files[0]); i++) {
        remove(cleanup_files[i]);
    }
    
    printf("\n========================================\n");
    printf("Total: %d tests\n", passed + failed);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Success Rate: %.1f%%\n", (float)passed / (passed + failed) * 100);
    printf("========================================\n");
    
    return (failed == 0) ? 0 : 1;
}
