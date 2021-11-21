CC = gcc
CCFLAGS = -Wall -Wextra -std=c99

SRC = bfc.c
NAME = bfc

BINDIR = bin

EXAMPLESDIR = examples
EXAMPLES = $(shell find $(EXAMPLESDIR) -name '*.bf' -type f)
EXAMPLES_NAME = $(EXAMPLES:$(EXAMPLESDIR)/%.bf=$(BINDIR)/%)

all: $(NAME) $(EXAMPLES_NAME)

$(NAME): bfc.c
	$(CC) $(CCFLAGS) -o $@ $<

$(BINDIR)/%: $(EXAMPLESDIR)/%.bf $(BINDIR)
	./$(NAME) -o $@ $<

$(BINDIR):
	mkdir -pv $@

clean:
	- rm -rv $(BINDIR)
	- rm $(NAME)

re: clean all

format:
	clang-format -i bfc.c

# weird GNU make behavior where it rm's every asm files at the end
# (https://stackoverflow.com/questions/47447369)
.PRECIOUS: $(EXAMPLES_ASM)
