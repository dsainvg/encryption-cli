#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int hash_password(const char *password, unsigned char *hash, size_t *hash_len);

int encrypt_file(const char *input_file, const char *output_file, const unsigned char *key, size_t key_len);

int decrypt_file(const char *input_file, const char *output_file, const unsigned char *key, size_t key_len);

#ifdef __cplusplus
}
#endif
