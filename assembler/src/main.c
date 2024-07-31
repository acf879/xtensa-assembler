#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

struct assembly_line {
  char* label;
  char* instruction;
  char* reg_src_0;
  char* reg_src_1;
  char* reg_ret;
  char* imm;
};

struct assembly_code {
  size_t num_lines;
  struct assembly_line* lines;
};

enum REG_OR_IMM {
  REG,
  IMM,
  NEITHER
};

// CORE INSTRUCTIONS -- No code density
const char* instructions[] = {"addmi","addx2","addx4","addx8",
  "addi", "add","subx2","subx4","subx8","neg","abs","saltu","salt",
  "l8ui","l16si","l16ui","l32i","l32r","s8i","s16i","s32i","memw",
  "extw","callx0","call0","ret","j","jx","ball","bnall","bany",
  "bnone","bbci","bbc","bbsi","bbs","beqi","beqz","beq","bnez","bnei",
  "bne","bgez","bgeui","bgeu","bgei","bge","bltz","bltui","bltu",
  "blti","blt","movnez","movltz","movgez","moveqz","movi","and","or",
  "xor","extui","srli","srai","slli","src","sll","srl","sra","ssa8l",
  "ssa8b","ssai","ssr","ssl","nop","fsync","dsync","esync","rsync",
  "isync","wur","rur","xsr","wsr","rsr","END"};

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

char* _grab_next_line(char* line_buff, const size_t max_line_size, FILE* assembly_file) {
  if (fgets(line_buff, max_line_size, assembly_file) == NULL) {
        return (char*)NULL;
  }
  size_t count=0;
  while (count<max_line_size) {
    if ((line_buff[count] == '\0') || (line_buff[count] == ';')) {
      break; 
    } 
    count++;
  }
  char temp_asm_buff[count];
  if (strncpy(temp_asm_buff, line_buff, count-1) == NULL) {
    return (char*)NULL;
  }
  temp_asm_buff[count-1] = '\0';
  if (strncpy(line_buff, temp_asm_buff, count) == NULL) {
    return (char*)NULL;
  }
  return line_buff;
}

char* cast_line_to_lowercase(char* line_to_parse) {
  const char* str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for (size_t i=0; i<strlen(line_to_parse); i++) {
    if (line_to_parse[i] == '\0') {
      break;
    }
    if (memchr(str, line_to_parse[i], strlen(str)) == NULL) {
      continue;
    }
    line_to_parse[i] = tolower(line_to_parse[i]);
  }
  return line_to_parse;
}

char* _extract_immediate_and_registers(const char* reg_or_imm_unparsed, char* reg_ret_or_src, char* reg_src_or_imm, char* reg_or_imm) {
  size_t cursor_1 = 0;
  size_t cursor_2 = 0;
  if (strncmp(reg_or_imm_unparsed, "", 1) == 0) {
    return (char*)reg_or_imm_unparsed;
  }
  char* next_comma = strchr(reg_or_imm_unparsed, ',');
  cursor_1 = cursor_2;
  if (next_comma != NULL) {
    cursor_2 = strlen(reg_or_imm_unparsed) - strlen(next_comma) + 1;
  } else {
    cursor_2 = strlen(reg_or_imm_unparsed);
  }
  if (next_comma == NULL) {
    if (strncpy(reg_ret_or_src, reg_or_imm_unparsed+cursor_1, cursor_2 - cursor_1) == NULL) {
      return (char*)NULL;
    }
    return reg_ret_or_src;
  }
  cursor_2 = strlen(reg_or_imm_unparsed) - strlen(next_comma) + 1;
  if (strncpy(reg_ret_or_src, reg_or_imm_unparsed+cursor_1, cursor_2 - cursor_1) == NULL) {
    return (char*)NULL;
  }
  next_comma = strchr(reg_or_imm_unparsed+cursor_2, ',');
  cursor_1 = cursor_2;
  if (next_comma != NULL) {
    cursor_2 = strlen(reg_or_imm_unparsed) - strlen(next_comma) + 1;
  } else {
    cursor_2 = strlen(reg_or_imm_unparsed);
  }
  if (next_comma == NULL) {
    if (strncpy(reg_src_or_imm, reg_or_imm_unparsed+cursor_1, cursor_2 - cursor_1) == NULL) {
      return (char*)NULL;
    }
    return reg_src_or_imm;
  }
  if (strncpy(reg_src_or_imm, reg_or_imm_unparsed+cursor_1, cursor_2 - cursor_1) == NULL) {
    return (char*)NULL;
  }
  next_comma = strchr(reg_or_imm_unparsed+cursor_2, ',');
  cursor_1 = cursor_2;
  if (next_comma != NULL) {
    cursor_2 = strlen(reg_or_imm_unparsed) - strlen(next_comma) + 1;
  } else {
    cursor_2 = strlen(reg_or_imm_unparsed);
  }
  if (next_comma == NULL) {
    if (strncpy(reg_or_imm, reg_or_imm_unparsed+cursor_1, cursor_2 - cursor_1) == NULL) {
      return (char*)NULL;
    }
    return reg_or_imm;
  }
  if (strncpy(reg_or_imm, reg_or_imm_unparsed+cursor_1, cursor_2 - cursor_1) == NULL) {
    return (char*)NULL;
  }
  return (char*)reg_or_imm_unparsed;
}

struct assembly_line* _extract_instructions_and_labels(char* line_to_parse, struct assembly_line* code_line) {
  size_t iter = 0;
  bool instruct_already_found = false;
  while (strncmp(instructions[iter], "END", 3) != 0) {
    if (instruct_already_found) {
      break;
    }
    line_to_parse = cast_line_to_lowercase(line_to_parse); // grab any way the instructions typed 
    char asm_instruction[strlen(instructions[iter])];
    char registers_and_imm[strlen(line_to_parse)-strlen(instructions[iter])+2]; // + 1 to include term char that is removed
    char* last_label_char_ptr = strchr(line_to_parse, ':');
    if (last_label_char_ptr != NULL) {
     char label[strlen(line_to_parse)-strlen(last_label_char_ptr)+2];
     if (strncpy(label, line_to_parse, strlen(line_to_parse)-strlen(last_label_char_ptr)+1) == NULL) { 
       return (struct assembly_line*)NULL;
     }
     if (label[strlen(label)-2] == ':') { 
       label[strlen(label)-2] = '\0'; // get rid of ':' char for label on line with instruction
     } else if (label[strlen(label)-1] == ':') {
       label[strlen(label)-1] = '\0'; // get rid of ':' char for label on seperate line
     }
     line_to_parse = last_label_char_ptr+1;
     if (strncpy(code_line->label, label, strlen(label)) == NULL) {
        return (struct assembly_line*)NULL;
     }
     if (strncmp(line_to_parse, "", 1) == 0) {
        return code_line;
     }
    }

    char* substr = strstr(line_to_parse, instructions[iter]);
    if (substr == NULL) {
      iter++;
      continue;
    }
    instruct_already_found = true;
    if (strncpy(asm_instruction, substr, strlen(instructions[iter])) == NULL) {
      return (struct assembly_line*)NULL;
    }
    asm_instruction[strlen(instructions[iter])] = '\0'; 
    if (strncpy(registers_and_imm, line_to_parse+strlen(asm_instruction), strlen(line_to_parse)-strlen(asm_instruction)+1) == NULL) {
      return (struct assembly_line*)NULL;
    }
    // instructions are now parsed out
    if (strncpy(code_line->instruction, asm_instruction, strlen(asm_instruction)) == NULL) {
        return (struct assembly_line*)NULL;
    }
    // Grab the registers dynamically and check for if the last is immediate or a reg
    char reg_ret_or_src[4]; // ret reg
    char reg_src_or_imm[11]; // src reg
    char reg_or_imm[11];
    if (_extract_immediate_and_registers(registers_and_imm, reg_ret_or_src, reg_src_or_imm, reg_or_imm) == NULL) {
      return (struct assembly_line*)NULL;
    }
    /** CHECK INSTRUCTION TO SEE IF WHAT IS AN IMM OR A REG THEN PACK THE ASSEMBLY LINE STRUCT
     *
     */
    iter++; 
  }
  return code_line;
}

FILE* _translate_line(char* line_buff, const size_t line_len, FILE* bin_file) {
  char line_buff_no_white_space[line_len];
  size_t cursor = 0;
  {
    size_t count = 0; 
    while (count < (line_len-1)) {
      if (line_buff[count] == ' ') { // remove all spaces (will look for first instance of instruction with commas acting as seperators following this)
        count++;
        continue;
      }
      if (line_buff[count] == '\0') {
        break;
      }
      line_buff_no_white_space[cursor] = line_buff[count];
      count++;
      cursor++;
    }
  }
  line_buff_no_white_space[cursor] = '\0';
  struct assembly_line line_parsed;
  char instructions[16];
  char label[128];
  line_parsed.label = label;
  line_parsed.instruction = instructions;
  if(_extract_instructions_and_labels(line_buff_no_white_space, &line_parsed) == NULL) {
    return (FILE*)NULL;
  }
  return bin_file;
}

bool _translate_no_opt(FILE* asm_file, FILE* bin_file) {
  const size_t line_len = 128;
  char asm_buffer[line_len];

  while (true) {
    if (_grab_next_line(asm_buffer, line_len, asm_file) == NULL) {
      return false;
    }
    if (strncmp(asm_buffer, "end", 3) == 0) {
      break;
    }
    if (_translate_line(asm_buffer, line_len, bin_file) == NULL) {
      return false;
    }
  }
  return true;
}

bool assemble_binary_file(const char* assembler_file_name,
    const char* bin_file_name) {
  FILE* bin_file = fopen(bin_file_name, "wb");
  FILE* assembly_file = fopen(assembler_file_name, "r");

  if (!_translate_no_opt(assembly_file, bin_file)) {
    fclose(bin_file);
    fclose(assembly_file);
    return false;
  }

  fclose(bin_file);
  fclose(assembly_file);
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
