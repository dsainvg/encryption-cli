#include "utils.h"
#include <stdio.h>
#include <string.h>

int file_exists(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

char *get_output_filename(const char *input_file, const char *suffix) {
    static char output[256];
    snprintf(output, sizeof(output), "%s%s", input_file, suffix);
    return output;
}
