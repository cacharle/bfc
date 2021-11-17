# BrainFuck Compiler (bfc)

Compiles [Brainfuck][1] to assembly and compile it to machine code with [`nasm`][2].

## Usage

```
$ make
$ ./bfc < ./brainfuck.bf > ./brainfuck.asm
```

You can then compile the assembly with the following:

```
$ nasm -f elf64 -o brainfuck.o brainfuck.asm 
$ ld -o brainfuck brainfuck.o
```

Use `-f macho64` instead of `-f elf64` if you're on MacOS.

## Examples

There is a few examples of brainfuck in the [examples directory](./examples/) (taken from Wikipedia).
After `make`, their executables are put in the `bin/` directory.

[1]: https://en.wikipedia.org/wiki/Brainfuck
[2]: https://nasm.us/
