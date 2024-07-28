#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

bool generate_empty_binary(const char* assembler_file_name, 
    char* bin_file_name) {
  size_t assembler_file_name_len = strlen(assembler_file_name);
  if (assembler_file_name_len < 3) {
    return false;
  }
  if (strncpy(bin_file_name, assembler_file_name, assembler_file_name_len-3) == NULL) {
    return false;
  }
  if (strncat(bin_file_name, "o", assembler_file_name_len-2) == NULL) {
    return false;
  }
  FILE* file = fopen(bin_file_name, "wb");
  if (file == NULL) {
    return false;
  }
  fclose(file);
  return true;
}

bool assemble_binary_file(const char* assembler_file_name,
    const char* bin_file_name) {
  return true;
}

int main(int argc, char *argv[]) {
  (void)argc;
  if (argv[1] == NULL) {
    return 1;
  }
  size_t assembler_file_name_len = strlen(argv[1]);
  if (assembler_file_name_len < 3) {
    return 1;
  }
  char bin_file_name[assembler_file_name_len-2];
  if (!generate_empty_binary(argv[1], bin_file_name)) {
    return 1;
  }
  if (!assemble_binary_file(argv[1], bin_file_name)) {
    return 1;
  }
  return 0;
}
