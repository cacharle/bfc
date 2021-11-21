# BrainFuck Compiler (bfc)

Compiles [Brainfuck][1] to assembly and compile it to machine code with [`nasm`][2].

## Usage

```
$ make
$ ./bfc -h
Usage: bfc [-h] [-S] [-o file] [-b buffer_len] [-e buffer_elem_bytes] [INPUT_FILE]

INPUT_FILE  Brainfuck source (read stdin if not present)

-h          Print this message
-S          Output assembly instead of compiled executable
-o          Output filename ('-' for stdout)
-b          Length of the buffer available the program
-e          Number of bytes in each element of the program's buffer
            (can only be: 1, 2, 4 or 8)

$ ./bfc -o hello_world examples/hello_world.bf
$ ./hello_world
Hello World!
```

## Examples

There is a few examples of brainfuck in the [examples directory](./examples/) (taken from Wikipedia).
After `make`, their executables are put in the `bin/` directory.

[1]: https://en.wikipedia.org/wiki/Brainfuck
[2]: https://nasm.us/
