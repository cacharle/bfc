#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

const size_t buffer_size = 256;
const char* asm_filename = "out.asm";

int main(int argc, char *argv[])
{
    FILE* file = fopen(asm_filename, "w");
    fprintf(
        file,
        "section .bss\n"
        "buffer: resb %zu\n"
        "section .text\n",
        "mov rax buffer\n",
        buffer_size
    );

    char c;
    while ((c = fgetc(file)) != EOF)
    {
        switch (c)
        {
        case '>':
            fputs("inc rax");
            break;
        case '<':
            fputs("dec rax");
            break;
        case '+':
            fputs("inc [rax]");
            break;
        case '-':
            fputs("dec [rax]");
            break;
        case '.':
            // putchar 
            break;
        case ',':
            // getchar
            break;
        case '[':
            // jump to next ] if byte at data ptr is 0
            break;
        case '[':
            // jump to prev [ if byte at data ptr is not 0
            break;
        case ';':
            // comment
            break;
        default:
            // error
        }
    }


    return 0;
}
