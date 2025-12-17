#include "../include/crypto.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int passed = 0, failed = 0;

static void test(const char *name, bool condition) {
    printf("Test %d: %s - %s\n", passed + failed + 1, name, condition ? "PASS" : "FAIL");
    if (condition) passed++; else failed++;
}

int main(void) {
    const char *salt1 = "testSalt12345678";
    const char *salt2 = "otherSalt1234567";
    
    // Test 1-2: Deterministic hashing
    char *h1 = hash_password("mypassword", 10, salt1);
    char *h2 = hash_password("mypassword", 10, salt1);
    test("Same password+salt produces same hash", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("test123", 8, salt1);
    h2 = hash_password("test123", 8, salt1);
    test("Deterministic at cost=8", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    // Test 3-5: Different passwords
    h1 = hash_password("password1", 10, salt1);
    h2 = hash_password("password2", 10, salt1);
    test("Different passwords produce different hashes", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("A", 10, salt1);
    h2 = hash_password("B", 10, salt1);
    test("Single char passwords differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("VeryLongPassword123456789", 10, salt1);
    h2 = hash_password("VeryLongPassword123456788", 10, salt1);
    test("Long similar passwords differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 6-8: Different salts
    h1 = hash_password("mypassword", 10, salt1);
    h2 = hash_password("mypassword", 10, salt2);
    test("Different salts produce different hashes", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 10, "a");
    h2 = hash_password("test", 10, "b");
    test("Single char salts differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 10, "salt123456789012");
    h2 = hash_password("test", 10, "salt123456789013");
    test("Similar salts differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 9-10: Random salts
    h1 = hash_password("mypassword", 10, NULL);
    h2 = hash_password("mypassword", 10, NULL);
    test("Random salts produce different hashes", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 8, NULL);
    h2 = hash_password("test", 8, NULL);
    test("Random salts at cost=8", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 11-14: Different cost factors
    h1 = hash_password("mypassword", 8, salt1);
    h2 = hash_password("mypassword", 10, salt1);
    test("Cost 8 vs 10 differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("mypassword", 10, salt1);
    h2 = hash_password("mypassword", 12, salt1);
    test("Cost 10 vs 12 differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 8, salt1);
    test("Cost=8 produces output", h1 && strlen(h1) > 20);
    free(h1);
    
    h1 = hash_password("test", 12, salt1);
    test("Cost=12 produces output", h1 && strlen(h1) > 20);
    free(h1);
    
    // Test 15-17: Case sensitivity
    h1 = hash_password("SecurePass2024!", 10, salt1);
    h2 = hash_password("securepass2024!", 10, salt1);
    test("Case sensitive hashing", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("ABC", 10, salt1);
    h2 = hash_password("abc", 10, salt1);
    test("Uppercase vs lowercase", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("Test123", 10, salt1);
    h2 = hash_password("test123", 10, salt1);
    test("Mixed case sensitivity", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 18-20: Special characters
    h1 = hash_password("pass@word", 10, salt1);
    h2 = hash_password("password", 10, salt1);
    test("Special chars matter", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("!@#$%^&*()", 10, salt1);
    h2 = hash_password("!@#$%^&*()", 10, salt1);
    test("Special chars deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("test space", 10, salt1);
    h2 = hash_password("testspace", 10, salt1);
    test("Spaces matter", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 21-23: Numeric passwords
    h1 = hash_password("123456", 10, salt1);
    h2 = hash_password("654321", 10, salt1);
    test("Numeric passwords differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("000000", 10, salt1);
    h2 = hash_password("111111", 10, salt1);
    test("Repeated digits differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("12345", 10, salt1);
    h2 = hash_password("12346", 10, salt1);
    test("Similar numbers differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 24-26: Empty and short inputs
    h1 = hash_password("", 10, salt1);
    test("Empty password handled", h1 && strlen(h1) > 0);
    free(h1);
    
    h1 = hash_password("a", 10, salt1);
    h2 = hash_password("a", 10, salt1);
    test("Single char deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("ab", 10, salt1);
    h2 = hash_password("ba", 10, salt1);
    test("Character order matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 27-29: Hash output format
    h1 = hash_password("test", 10, salt1);
    test("Hash contains salt in output", h1 && strstr(h1, salt1) != NULL);
    test("Hash contains separator", h1 && strstr(h1, "$/$") != NULL);
    test("Hash length reasonable", h1 && strlen(h1) > 30 && strlen(h1) < 2000);
    free(h1);
    
    // Test 30-32: Consistency across runs
    h1 = hash_password("consistent", 10, salt1);
    h2 = hash_password("consistent", 10, salt1);
    char *h3 = hash_password("consistent", 10, salt1);
    test("Consistent across 3 runs", h1 && h2 && h3 && strcmp(h1, h2) == 0 && strcmp(h2, h3) == 0);
    free(h1); free(h2); free(h3);
    
    h1 = hash_password("test", 9, salt1);
    h2 = hash_password("test", 9, salt1);
    test("Cost=9 deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 11, salt1);
    h2 = hash_password("test", 11, salt1);
    test("Cost=11 deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    // Test 33-35: Edge cases with salts
    h1 = hash_password("test", 10, "");
    test("Empty salt handled", h1 && strlen(h1) > 0);
    free(h1);
    
    h1 = hash_password("test", 10, "verylongsaltstring1234567890");
    test("Long salt handled", h1 && strlen(h1) > 0);
    free(h1);
    
    h1 = hash_password("test", 10, "!@#$%^&*()");
    test("Special char salt handled", h1 && strlen(h1) > 0);
    free(h1);
    
    // Test 36-38: Repeated characters
    h1 = hash_password("aaaaaaaaaa", 10, salt1);
    h2 = hash_password("bbbbbbbbbb", 10, salt1);
    test("Repeated chars differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("aaa", 10, salt1);
    h2 = hash_password("aaaa", 10, salt1);
    test("Length matters for repeated chars", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("abcabc", 10, salt1);
    h2 = hash_password("abcabcd", 10, salt1);
    test("Pattern length matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 39-40: Mixed alphanumeric
    h1 = hash_password("Pass123Word456", 10, salt1);
    h2 = hash_password("Pass123Word456", 10, salt1);
    test("Alphanumeric deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("abc123xyz", 10, salt1);
    h2 = hash_password("xyz123abc", 10, salt1);
    test("Alphanumeric order matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 41-45: Unicode and extended characters
    h1 = hash_password("café", 11, salt1);
    h2 = hash_password("cafe", 11, salt1);
    test("Unicode chars differ from ASCII", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test\ttab", 10, salt1);
    h2 = hash_password("test tab", 10, salt1);
    test("Tab vs space differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("line1\nline2", 11, salt1);
    h2 = hash_password("line1line2", 11, salt1);
    test("Newline matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test@#$%", 10, salt1);
    h2 = hash_password("test@#$%", 10, salt1);
    test("Complex special chars deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("[]{}<>", 11, salt1);
    h2 = hash_password("[]{}><", 11, salt1);
    test("Bracket order matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 46-50: Password length variations
    h1 = hash_password("short", 10, salt1);
    h2 = hash_password("verylongpasswordthatgoeson", 10, salt1);
    test("Short vs long passwords differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("12345678901234567890", 11, salt1);
    h2 = hash_password("12345678901234567890", 11, salt1);
    test("20-char password deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("a", 12, salt1);
    h2 = hash_password("aa", 12, salt1);
    test("Single vs double char", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("password", 10, salt1);
    h2 = hash_password("password ", 10, salt1);
    test("Trailing space matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password(" password", 10, salt1);
    h2 = hash_password("password", 10, salt1);
    test("Leading space matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 51-55: Salt variations with different costs
    h1 = hash_password("test", 10, "salt1");
    h2 = hash_password("test", 11, "salt1");
    test("Same salt different cost", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 12, "a");
    h2 = hash_password("test", 12, "a");
    test("Single char salt at cost=12", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 13, salt1);
    h2 = hash_password("test", 13, salt1);
    test("Cost=13 deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 10, "123");
    h2 = hash_password("test", 10, "456");
    test("Numeric salts differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 11, "!@#");
    h2 = hash_password("test", 11, "$%^");
    test("Special char salts differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 56-60: Boundary conditions
    h1 = hash_password("0", 10, salt1);
    h2 = hash_password("1", 10, salt1);
    test("Single digit passwords differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("!", 11, salt1);
    h2 = hash_password("?", 11, salt1);
    test("Single special char differs", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 10, "x");
    h2 = hash_password("test", 10, "y");
    test("Single letter salts differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("AAA", 12, salt1);
    h2 = hash_password("ZZZ", 12, salt1);
    test("Extreme letter differences", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("000", 10, salt1);
    h2 = hash_password("999", 10, salt1);
    test("Extreme digit differences", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 61-65: Mixed complexity
    h1 = hash_password("Aa1!", 10, salt1);
    h2 = hash_password("Aa1!", 10, salt1);
    test("Mixed complexity deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("Pass123!@#", 11, salt1);
    h2 = hash_password("Pass123!@$", 11, salt1);
    test("Similar complex passwords differ", !(h1 && h2 && strcmp(h1, h2) != 0));
    free(h1); free(h2);
    
    h1 = hash_password("test", 10, "MixedCase123");
    h2 = hash_password("test", 10, "mixedcase123");
    test("Salt case sensitivity", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("123abc!@#", 12, salt1);
    h2 = hash_password("123abc!@#", 12, salt1);
    test("Complex at cost=12 deterministic", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("a1b2c3", 10, salt1);
    h2 = hash_password("c3b2a1", 10, salt1);
    test("Interleaved alphanumeric order", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 66-70: Repetition patterns
    h1 = hash_password("abcabcabc", 11, salt1);
    h2 = hash_password("abcabcab", 11, salt1);
    test("Pattern repetition length", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("121212", 10, salt1);
    h2 = hash_password("212121", 10, salt1);
    test("Alternating patterns differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("aabbcc", 11, salt1);
    h2 = hash_password("aabbc", 11, salt1);
    test("Paired pattern length", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("xyz", 10, "abcabcabc");
    h2 = hash_password("xyz", 10, "abcabcab");
    test("Salt pattern length matters", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 12, "aaa");
    h2 = hash_password("test", 12, "bbb");
    test("Repeated salt chars differ", h1 && h2 && strcmp(h1, h2) != 0);
    free(h1); free(h2);
    
    // Test 71-75: Hash output properties
    h1 = hash_password("test", 10, salt1);
    test("Hash starts with $", h1 && h1[0] == '$');
    free(h1);
    
    h1 = hash_password("test", 11, salt1);
    size_t len = strlen(h1);
    test("Cost=11 produces reasonable length", len > 50 && len < 1500);
    free(h1);
    
    h1 = hash_password("test", 12, salt1);
    len = strlen(h1);
    test("Cost=12 produces reasonable length", len > 50 && len < 1500);
    free(h1);
    
    h1 = hash_password("test", 13, salt1);
    len = strlen(h1);
    test("Cost=13 produces reasonable length", len > 50 && len < 1500);
    free(h1);
    
    h1 = hash_password("unique", 10, salt1);
    h2 = hash_password("unique", 10, salt1);
    test("Same input same hash", h1 && h2 && strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    // Test 76-80: Cross-cost consistency
    h1 = hash_password("consistent", 10, salt1);
    h2 = hash_password("consistent", 10, salt1);
    h3 = hash_password("consistent", 10, salt1);
    char *h4 = hash_password("consistent", 10, salt1);
    test("Consistent across 4 runs", strcmp(h1, h2) == 0 && strcmp(h2, h3) == 0 && strcmp(h3, h4) == 0);
    free(h1); free(h2); free(h3); free(h4);
    
    h1 = hash_password("test", 10, "salt123");
    h2 = hash_password("test", 10, "salt123");
    test("Short salt deterministic", strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("test", 11, "longsaltstring12345");
    h2 = hash_password("test", 11, "longsaltstring12345");
    test("Long salt deterministic", strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("P@ssw0rd!", 12, salt1);
    h2 = hash_password("P@ssw0rd!", 12, salt1);
    test("Complex password at cost=12", strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    h1 = hash_password("final_test_123", 13, salt1);
    h2 = hash_password("final_test_123", 13, salt1);
    test("Final test at cost=13", strcmp(h1, h2) == 0);
    free(h1); free(h2);
    
    printf("\n========================================\n");
    printf("Total: %d tests\n", passed + failed);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Success Rate: %.1f%%\n", (float)passed / (passed + failed) * 100);
    printf("========================================\n");
    
    return (failed == 0) ? 0 : 1;
}
