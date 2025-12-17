#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Byte manipulation functions
void rotate_left(uint8_t *data, size_t len, int k);
void rotate_right(uint8_t *data, size_t len, int k);
void xor_bytes(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len);
void byte_manipulations(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len, int iterat);
void byte_manipulations_reverse(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len, int iterat);

// File encryption/decryption with advanced features
int encrypt_file_advanced(const char *input_file, const char *output_file, const char *password, int cost);
int decrypt_file_advanced(const char *input_file, const char *output_file, const char *password);

#ifdef __cplusplus
}
#endif
