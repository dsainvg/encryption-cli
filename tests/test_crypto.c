#include "crypto.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    unsigned char hash[64];
    size_t hash_len = sizeof(hash);
    
    int rc = hash_password("testpass", hash, &hash_len);
    if (rc != 0) {
        printf("hash_password failed\n");
        return 1;
    }
    
    printf("Password hash: ");
    for (size_t i = 0; i < hash_len; ++i) {
        printf("%02X ", hash[i]);
    }
    printf("\n");
    
    return 0;
}
