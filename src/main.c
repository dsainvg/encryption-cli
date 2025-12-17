#include "cli.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <hash|encrypt|decrypt> <password|filepath> [password] [output_file]\n", argv[0]);
        return 1;
    }
    
    // Handle hash command
    if (strcmp(argv[1], "hash") == 0) {
        if (argc < 3) {
            printf("Usage: %s hash <password>\n", argv[0]);
            return 1;
        }
        unsigned char hash[64];
        size_t hash_len = sizeof(hash);
        if (hash_password(argv[2], hash, &hash_len) != 0) {
            printf("Error hashing password\n");
            return 1;
        }
        printf("Hash: ");
        for (size_t i = 0; i < hash_len; i++) {
            printf("%02x", hash[i]);
        }
        printf("\n");
        return 0;
    }
    
    if (argc < 4) {
        printf("Usage: %s <encrypt|decrypt> <filepath> <password> [output_file]\n", argv[0]);
        return 1;
    }
    
    CliArgs args;
    if (parse_args(argc, argv, &args) != 0) {
        printf("Error parsing arguments\n");
        return 1;
    }
    
    if (strcmp(args.command, "encrypt") != 0 && strcmp(args.command, "decrypt") != 0) {
        printf("Invalid command: %s\n", args.command);
        return 1;
    }
    
    unsigned char hash[64];
    size_t hash_len = sizeof(hash);
    if (hash_password(args.password, hash, &hash_len) != 0) {
        printf("Error hashing password\n");
        return 1;
    }
    
    char output_file[256];
    if (args.output_file) {
        strcpy(output_file, args.output_file);
    } else {
        if (strcmp(args.command, "encrypt") == 0) {
            snprintf(output_file, sizeof(output_file), "%s.enc", args.filepath);
        } else {
            snprintf(output_file, sizeof(output_file), "%s.dec", args.filepath);
        }
    }
    
    int rc;
    if (strcmp(args.command, "encrypt") == 0) {
        rc = encrypt_file(args.filepath, output_file, hash, hash_len);
        if (rc == 0) {
            printf("File encrypted: %s\n", output_file);
        } else {
            printf("Encryption failed\n");
        }
    } else if (strcmp(args.command, "decrypt") == 0) {
        rc = decrypt_file(args.filepath, output_file, hash, hash_len);
        if (rc == 0) {
            printf("File decrypted: %s\n", output_file);
        } else {
            printf("Decryption failed\n");
        }
    } else {
        printf("Unknown command: %s\n", args.command);
        return 1;
    }
    
    free_args(&args);
    return rc;
}
