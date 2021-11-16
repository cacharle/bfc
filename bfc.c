#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static const size_t buffer_size = 256;
static const char* asm_filename = "out.asm";

static const char *write_syscall = "0x2000004";
static const char *read_syscall  = "0x2000003";

#define LABEL_COUNT_STACK_SIZE 256
static size_t label_stack[LABEL_COUNT_STACK_SIZE + 1] = {0};
static size_t label_stack_frame = 0;

int main(int argc, char *argv[])
{
    FILE *input_file = NULL;
    if (argc == 2)
        input_file = fopen(argv[1], "r");
    else
        input_file = stdin;

    /* FILE* output_file = fopen(asm_filename, "w"); */
    FILE *output_file = stdout;
    fprintf(
        output_file,
        "global _start\n"
        "section .bss\n"
        "\tbuffer: resb %zu\n\n"
        "section .text\n"
        "_start:\n"
        "\tmov rbx, buffer\n",
        buffer_size
    );
    size_t label_frame_id;

    char c;
    while ((c = fgetc(input_file)) != EOF)
    {
        switch (c)
        {
        case '>':
            fprintf(output_file, "    inc rbx              ; >\n");
            break;
        case '<':
            fprintf(output_file, "    dec rbx              ; <\n");
            break;
        case '+':
            fprintf(output_file, "    inc byte [rbx]       ; +\n");
            break;
        case '-':
            fprintf(output_file, "    dec byte [rbx]       ; -\n");
            break;
        case '.':
            fprintf(output_file, "    mov rdi, 1           ; .\n");
            fprintf(output_file, "    mov rsi, rbx         ; .\n");
            fprintf(output_file, "    mov rdx, 1           ; .\n");
            fprintf(output_file, "    mov rax, %-10s  ; .\n", write_syscall);
            fprintf(output_file, "    syscall              ; .\n");
            break;
        case ',':
            fprintf(output_file, "    mov rdi, 0           ; ,\n");
            fprintf(output_file, "    mov rsi, rbx         ; ,\n");
            fprintf(output_file, "    mov rdx, 1           ; ,\n");
            fprintf(output_file, "    mov rax, %-10s  ; ,\n", read_syscall);
            fprintf(output_file, "    syscall              ; ,\n");
            break;
        case '[':
            label_frame_id = label_stack[label_stack_frame];
            fprintf(output_file, "label_open_%03d_%03d:          ; [\n", label_stack_frame, label_frame_id);
            fprintf(output_file, "    cmp byte [rbx], 0    ; [\n");
            fprintf(output_file, "    je  label_close_%03d_%03d  ; [\n", label_stack_frame, label_frame_id);
            label_stack_frame++;
            break;
        case ']':
            label_stack_frame--;
            label_frame_id = label_stack[label_stack_frame];
            fprintf(output_file, "label_close_%03d_%03d:         ; ]\n", label_stack_frame, label_frame_id);
            fprintf(output_file, "    cmp byte [rbx], 0    ; ]\n");
            fprintf(output_file, "    jne label_open_%03d_%03d   ; ]\n", label_stack_frame, label_frame_id);
            label_stack[label_stack_frame]++;
            break;
        case ';':
            while (c != EOF && c != '\n')
                c = fgetc(input_file);
            break;
        default:
            // error
        }
    }


    return 0;
}
