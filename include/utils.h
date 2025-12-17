#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int file_exists(const char *filepath);
char *get_output_filename(const char *input_file, const char *suffix);

#ifdef __cplusplus
}
#endif
