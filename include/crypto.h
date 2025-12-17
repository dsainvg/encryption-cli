#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char* hash_password(const char *password, int cost, const char *salt);

int encrypt_file(const char *input_file, const char *output_file, const unsigned char *key, size_t key_len);

int decrypt_file(const char *input_file, const char *output_file, const unsigned char *key, size_t key_len);

#ifdef __cplusplus
}
#endif
