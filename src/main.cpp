#include "cli.h"
#include "crypto.h"
#include "encryption.h"
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
            printf("Usage: %s hash <password> [cost] [salt]\n", argv[0]);
            return 1;
        }
        int cost = (argc >= 4) ? atoi(argv[3]) : 10;
        const char *salt = (argc >= 5) ? argv[4] : NULL;
        char *hash = hash_password(argv[2], cost, salt);
        if (!hash) {
            printf("Error hashing password\n");
            return 1;
        }
        printf("Hash: %s\n", hash);
        free(hash);
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
        rc = encrypt_file_advanced(args.filepath, output_file, args.password, 10);
        if (rc == 0) {
            printf("File encrypted: %s\n", output_file);
        } else {
            printf("Encryption failed\n");
        }
    } else if (strcmp(args.command, "decrypt") == 0) {
        rc = decrypt_file_advanced(args.filepath, output_file, args.password);
        if (rc == 0) {
            printf("File decrypted: %s\n", output_file);
        } else if (rc == -2) {
            printf("Decryption failed: Wrong password\n");
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
