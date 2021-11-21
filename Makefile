CC = gcc
CCFLAGS = -Wall -Wextra -std=c99
NASM = nasm

ifeq ($(shell uname),Linux)
	NASMFLAGS = -f elf64
endif
ifeq ($(shell uname),Darwin)
	NASMFLAGS = -f macho64
endif
ifeq ($(NASMFLAGS),)
	$(error architecture: $(shell uname) not recognized)
endif

SRC = bfc.c
NAME = bfc

ASMDIR = asm
BINDIR = bin

EXAMPLESDIR = examples
EXAMPLES = $(shell find $(EXAMPLESDIR) -name '*.bf' -type f)
EXAMPLES_ASM = $(EXAMPLES:$(EXAMPLESDIR)/%.bf=$(ASMDIR)/%.asm)
EXAMPLES_NAME = $(EXAMPLES:$(EXAMPLESDIR)/%.bf=$(BINDIR)/%)

all: $(NAME) $(EXAMPLES_NAME)

$(NAME): bfc.c
	$(CC) $(CCFLAGS) -o $@ $<

$(BINDIR)/%: $(ASMDIR)/%.asm $(BINDIR)
	$(NASM) $(NASMFLAGS) -o $@.o $<
	ld -o $@ $@.o

$(ASMDIR)/%.asm: $(EXAMPLESDIR)/%.bf $(ASMDIR)
	./$(NAME) < $< > $@

$(BINDIR):
	mkdir -pv $@

$(ASMDIR):
	mkdir -pv $@

clean:
	- rm -rv $(ASMDIR)
	- rm -rv $(BINDIR)
	- rm $(NAME)

re: clean all

# weird GNU make behavior where it rm's every asm files at the end
# (https://stackoverflow.com/questions/47447369)
.PRECIOUS: $(EXAMPLES_ASM)
