# Natrix

## Dev-environment

Build:

```sh
docker build . -t zubrailx-natrix
```

Run container and execute commands inside:

```sh
docker run -v .:/workspace -it zubrailx-natrix:latest bash
```

Or run and connect with SSH:

```sh
docker run -v .:/workspace -itd zubrailx-natrix:latest

ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o GlobalKnownHostsFile=/dev/null \
    -p 2222 user@<ip>
```

- `ip` - container ip

Default password for user `user` is `user`.

Cd to mounted workspace:

```sh
cd /workspace
```

## Configuration

Make Targets:

```sh
make help
```

```
Targets:
  clean/build                 Remove build files
  clean/out                   Remove io output files
  compiler                    Build compiler executable
  debugger                    Build debugger executable
  format                      Format
  help                        Help
  index                       Index project
  run/asm                     Compile file
  run/dbg                     Run debugger
  run/pdf                     Run main executable and generate dot
  run/test                    Run all tests or specific
  test                        Build tests
  util                        Build libutil.a
  x86_64_core                 Build libx86_64_core.a
  x86_64_std                  Build libx86_64_std.a
Modules (prefixes of module targets):
  util compiler x86_64_core x86_64_std debugger test
```

### Build compiler

```sh
make compiler
```

### Build debugger

```sh
make debugger
```

### Test

```sh
make run/test
```

### Run assembly

Run default suite:

```sh
make run/asm
```

Run selected suite (for example `io/suite28.txt`):

```sh
make run/asm/io/suite28.txt
```

Run manually:

```sh
make x86_64_core x86_64_std

./build/compiler/main  \
        -o io/suite28.txt.asm \
        io/suite28.txt /workspace/src/x86_64_std/x86_64_std.txt

as --64 -g -o io/suite28.txt.asm.o io/suite28.txt.asm

ld -g -z noexecstack -o io/suite28.txt.asm.out \
        -dynamic-linker /usr/lib64/ld-linux-x86-64.so.2 \
        /usr/lib/x86_64-linux-gnu/crt1.o \
        /usr/lib/x86_64-linux-gnu/crti.o \
        -lc \
        io/suite28.txt.asm.o \
        /workspace/build/x86_64_core/libx86_64_core.a \
        /workspace/build/x86_64_std/libx86_64_std.a \
        /workspace/build/util/libutil.a \
        /usr/lib/x86_64-linux-gnu/crtn.o
```

Help:

```sh
./build/compiler/main -h
```

```
Usage: ./build/compiler/main [options] <file>...
Options:
-d <directory>   - output directory (current: .)
-o <file>        - main output file (current: a.asm)
--tee            - print to file and to stdout (current: 0)
--ignore-errors  - continue execution on errors (current: 0)
--ast            - add AST output (current: 0)
--cfg            - add global subroutines control flow graph output (current: 0)
--cfg-add-expr   - include expressions in control flow graph (current: 0)
--cg             - add global subroutines call graph output (current: 0)
-s <subroutine>  - set or add global subroutine for call graph generation (current: main)
--hir-tree       - print HIR tree (current: 0)
--hir-symbols    - print HIR symbol table (current: 0)
--hir-types      - print HIR type table (current: 0)
--mir            - print MIR tree (current: 0)
-h
--help           - show help
```

### Run debugger

Run with default init file:

```sh
make run/dbg
```

With specific init:

```sh
make run/dbg/io/dbg_init.txt
```

Run manually:

```sh
./build/debugger/main
```

Help:

```sh
./build/debugger/main -h
```

```
Usage: ./build/debugger/main [options] <file>...
Options:
-i <file>  - init file (current: (null))
-h         - show help
```
