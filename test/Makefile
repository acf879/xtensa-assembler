ASM = ../assembler
ASM_SRC = $(ASM)/src

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

.PHONY: all clean

BINARIES = ../assembler/asm-xtensa 

all: $(BINARIES)
	
clean: 
	rm -rf $(BINARIES) ../assembler/*.o

$(ASM)/main.o : $(ASM_SRC)/main.c | $(ASM)
	$(CC) $(CFLAGS) -c $< -o $@
	 

$(ASM)/asm-xtensa : $(ASM)/main.o | $(ASM)
	$(CC) $(CFLAGS) $< -o $@

edit_main_c:
	nvim ../assembler/src/main.c
