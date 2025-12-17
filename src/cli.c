#include "cli.h"
#include <stdlib.h>
#include <string.h>

int parse_args(int argc, char *argv[], CliArgs *args) {
    if (argc < 4) return -1;
    
    args->command = argv[1];
    args->filepath = argv[2];
    args->password = argv[3];
    args->output_file = (argc > 4) ? argv[4] : NULL;
    
    return 0;
}

void free_args(CliArgs *args) {
    // No dynamic allocation in this simple version
    (void)args;
}
