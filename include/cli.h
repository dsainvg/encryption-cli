#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *command;
    char *filepath;
    char *password;
    char *output_file;
} CliArgs;

int parse_args(int argc, char *argv[], CliArgs *args);

void free_args(CliArgs *args);

#ifdef __cplusplus
}
#endif
