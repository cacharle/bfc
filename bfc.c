#define _XOPEN_SOURCE 500
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __linux__
#define NASM_FORMAT "elf64"
#endif
#ifdef __APPLE__
#define NASM_FORMAT "macho64"
#endif
#ifndef NASM_FORMAT
#error "architecture not supported"
#endif

#define LABEL_COUNT_STACK_SIZE 4096
static size_t label_stack[LABEL_COUNT_STACK_SIZE + 1] = {0};
static size_t label_stack_frame = 0;

static bool   compile_to_exec = true;
static char  *output_filename = NULL;
static size_t buffer_len = 256;
static size_t buffer_elem_bytes = 1;

static char *elem_bytes_asm_str = "byte";
static char *elem_bytes_asm_str_res = "resb";

#define TMP_FILEPATH_TEMPLATE "/tmp/bfc_asm_XXXXXX"
#define TMP_OBJ_FILEPATH_TEMPLATE (TMP_FILEPATH_TEMPLATE ".o")
static char tmp_filepath[] = TMP_FILEPATH_TEMPLATE;
static char tmp_obj_filepath[] = TMP_OBJ_FILEPATH_TEMPLATE;

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fputs("bfc: ", stderr);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static size_t
xatoul(char *s)
{
    if (!isdigit(*s))
        die("'%s' is not a valid number", s);
    char  *end;
    size_t num = strtoul(s, &end, 10);
    if (*end != '\0')
        die("'%s' is not a valid number", s);
    return num;
}

static void
run_subprocess(char *name, char *const argv[])
{
    pid_t pid = fork();
    if (pid == -1)
        die("%s", strerror(errno));
    if (pid == 0)
    {
        execvp(name, argv);
        die("%s", strerror(errno));
    }
    waitpid(pid, &pid, 0);
    int status = WEXITSTATUS(pid);
    if (status != 0)
        die("%s process failed with %d\n", name, status);
}

static void
remove_tmp_file(void)
{
    if (strcmp(tmp_filepath, TMP_FILEPATH_TEMPLATE) != 0)
        remove(tmp_filepath);
    if (strcmp(tmp_obj_filepath, TMP_OBJ_FILEPATH_TEMPLATE) != 0)
        remove(tmp_obj_filepath);
}

int
main(int argc, char *argv[])
{
    int option;

    while ((option = getopt(argc, argv, "So:b:e:h")) != -1)
    {
        switch (option)
        {
        case 'S':
            compile_to_exec = false;
            break;
        case 'o':
            output_filename = optarg;
            break;
        case 'b':
            buffer_len = xatoul(optarg);
            break;
        case 'e':
            buffer_elem_bytes = xatoul(optarg);
            switch (buffer_elem_bytes)
            {
            case 1:
                elem_bytes_asm_str = "byte";
                elem_bytes_asm_str_res = "resb";
                break;
            case 2:
                elem_bytes_asm_str = "word";
                elem_bytes_asm_str_res = "resw";
                break;
            case 4:
                elem_bytes_asm_str = "dword";
                elem_bytes_asm_str_res = "resd";
                break;
            case 8:
                elem_bytes_asm_str = "qword";
                elem_bytes_asm_str_res = "resq";
                break;
            default:
                die("%lu: is not a valid number of bytes for buffer element", buffer_elem_bytes);
            }
            break;
        case 'h':
            puts("Usage: bfc [-h] [-S] [-o file] [-b buffer_len] [-e "
                 "buffer_elem_bytes] [INPUT_FILE]");
            puts("");
            puts("INPUT_FILE  Brainfuck source (read stdin if not present)");
            puts("");
            puts("-h          Print this message");
            puts("-S          Output assembly instead of compiled executable");
            puts("-o          Output filename ('-' for stdout)");
            puts("-b          Length of the buffer available the program");
            puts("-e          Number of bytes in each element of the program's "
                 "buffer");
            puts("            (can only be: 1, 2, 4 or 8)");
            exit(EXIT_SUCCESS);
        }
    }
    if (optind > argc + 1)
        die("Unexpected argument: %s\n", argv[optind + 1]);

    if (output_filename == NULL)
        output_filename = compile_to_exec ? "a.out" : "a.asm";

    atexit(remove_tmp_file);

    FILE *input_file = NULL;
    if (optind == argc)
        input_file = stdin;
    else
    {
        input_file = fopen(argv[optind], "r");
        if (input_file == NULL)
            die("%s: %s\n", strerror(errno), argv[optind]);
    }

    FILE *output_file = NULL;
    if (strcmp(output_filename, "-") == 0)
        output_file = stdout;
    else
    {
        if (compile_to_exec)
            output_file = fdopen(mkstemp(tmp_filepath), "w");
        else
            output_file = fopen(output_filename, "w");
        if (output_file == NULL)
            die("%s: %s\n", strerror(errno), argv[optind]);
    }

    fprintf(output_file,
            "global _start\n\n"
            "section .bss\n"
            "\tbuffer: %s %zu\n\n"
            "section .text\n"
            "_start:\n"
            "\tmov rbx, buffer\n",
            elem_bytes_asm_str_res,
            buffer_len);
    size_t label_frame_id;

    char c;
    while ((c = fgetc(input_file)) != EOF)
    {
        switch (c)
        {
        case '>':
            fprintf(output_file, "    add rbx, %zu                ; >\n", buffer_elem_bytes);
            break;
        case '<':
            fprintf(output_file, "    sub rbx, %zu                ; <\n", buffer_elem_bytes);
            break;
        case '+':
            fprintf(output_file, "    inc %s [rbx]           ; +\n", elem_bytes_asm_str);
            break;
        case '-':
            fprintf(output_file, "    dec %s [rbx]           ; -\n", elem_bytes_asm_str);
            break;
        case '.':
            fprintf(output_file, "    mov rdi, 1               ; .\n");
            fprintf(output_file, "    mov rsi, rbx             ; .\n");
            fprintf(output_file, "    mov rdx, 1               ; .\n");
            fprintf(output_file, "    mov rax, 1               ; .\n");
            fprintf(output_file, "    syscall                  ; .\n");
            break;
        case ',':
            fprintf(output_file, "    mov rdi, 0               ; ,\n");
            fprintf(output_file, "    mov rsi, rbx             ; ,\n");
            fprintf(output_file, "    mov rdx, 1               ; ,\n");
            fprintf(output_file, "    mov rax, 0               ; ,\n");
            fprintf(output_file, "    syscall                  ; ,\n");
            break;
        case '[':
            label_frame_id = label_stack[label_stack_frame];
            fprintf(output_file,
                    "label_open_%03zu_%03zu:          ; [\n",
                    label_stack_frame,
                    label_frame_id);
            fprintf(output_file, "    cmp %s [rbx], 0        ; [\n", elem_bytes_asm_str);
            fprintf(output_file,
                    "    je  label_close_%03zu_%03zu  ; [\n",
                    label_stack_frame,
                    label_frame_id);
            label_stack_frame++;
            break;
        case ']':
            label_stack_frame--;
            label_frame_id = label_stack[label_stack_frame];
            fprintf(output_file,
                    "label_close_%03zu_%03zu:         ; ]\n",
                    label_stack_frame,
                    label_frame_id);
            fprintf(output_file, "    cmp %s [rbx], 0        ; ]\n", elem_bytes_asm_str);
            fprintf(output_file,
                    "    jne label_open_%03zu_%03zu   ; ]\n",
                    label_stack_frame,
                    label_frame_id);
            label_stack[label_stack_frame]++;
            break;
        case ';':
            while (c != EOF && c != '\n')
                c = fgetc(input_file);
            break;
        default:
            break;
        }
    }
    if (input_file != stdin)
        fclose(input_file);
    fprintf(output_file, "\n");
    fprintf(output_file, "    mov rdi, 0   ; exit\n");
    fprintf(output_file, "    mov rax, 60  ; exit\n");
    fprintf(output_file, "    syscall      ; exit\n");
    if (output_file != stdout)
        fclose(output_file);

    if (compile_to_exec)
    {
        strcpy(tmp_obj_filepath, tmp_filepath);
        strcat(tmp_obj_filepath, ".o");
        char *const nasm_argv[] = {
            "nasm", "-f", NASM_FORMAT, "-o", tmp_obj_filepath, tmp_filepath, NULL};
        run_subprocess("nasm", nasm_argv);
        char *const ld_argv[] = {"ld", "-o", output_filename, tmp_obj_filepath, NULL};
        run_subprocess("ld", ld_argv);
    }

    return 0;
}
