# ft_ssl

`ft_ssl` is a C implementation of a subset of OpenSSL hashing commands, built for the 42 school `ft_ssl` project.
It reproduces the expected CLI behavior for `md5` and `sha256` (mandatory), and includes both bonus parts:
interactive command parsing from `STDIN` and `whirlpool`.

## Project goals

- Re-implement hashing algorithms from scratch (no crypto library calls)
- Match the subject output format and flag behavior
- Keep code modular for extension in future `ft_ssl` modules
- Handle errors safely and continue processing remaining inputs

## Implemented commands

| Command | Digest size | Status | Reference |
|---|---:|---|---|
| `md5` | 128-bit | Mandatory | RFC 1321 |
| `sha256` | 256-bit | Mandatory | FIPS 180-4 |
| `whirlpool` | 512-bit | Bonus | ISO/IEC 10118-3 |

## Supported flags

All hash commands support:

| Flag | Behavior |
|---|---|
| `-p` | Echo `STDIN` to `STDOUT`, then print digest |
| `-q` | Quiet mode: print only digest |
| `-r` | Reverse output format |
| `-s <str>` | Hash a string literal |

Behavior follows the subject examples, including mixed flag/file edge cases.

## Build

```bash
make
make clean
make fclean
make re
```

Compiler flags:

- `-Wall`
- `-Wextra`
- `-Werror`

Executable:

```bash
./ft_ssl
```

## Usage

```text
ft_ssl command [flags] [file/string]
```

Examples:

```bash
echo "42 is nice" | ./ft_ssl md5
# (stdin)= 35f1d6de0302e2086a4e472266efb3a9

./ft_ssl md5 -s "pity those that aren't following baerista on spotify."
# MD5 ("pity those that aren't following baerista on spotify.") = a3c990a1964705d9bf0e602f44572f5f

echo "https://www.42.fr/" > website
./ft_ssl sha256 -q website
# 1ceb55d2845d9dd98557b50488db12bbf51aaca5aa9c1199eb795607a2457daf
```

Invalid command example:

```bash
./ft_ssl foobar
# ft_ssl: Error: 'foobar' is an invalid command.
# Commands:
# md5
# sha256
# whirlpool
# Flags:
# -p -q -r -s
```

## Bonus features

### 1) Interactive parsing from STDIN

Running without arguments enters interactive mode after printing usage:

```bash
./ft_ssl
```

Then you can type commands:

```text
ft_ssl> md5 -s "hello"
MD5 ("hello") = 5d41402abc4b2a76b9719d911017c592
ft_ssl> sha256 -s "hello"
SHA256 ("hello") = 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824
```

### 2) Whirlpool implementation

Includes a full `whirlpool` command with verified known vectors such as:

- `""` -> `19fa61d75522a466...42a66eb3`
- `"a"` -> `8aca2602792aec6f...fdae231a`
- `"abc"` -> `4e2448a4c6f486bb...76d4eef5`

## Internal architecture

High-level layout:

- `srcs/main.c` - entry point and command bootstrap
- `srcs/dispatcher.c` - command registry and dispatch
- `srcs/execute.c` - flag/argument parsing and execution order
- `srcs/output.c` - formatting and output policy
- `srcs/md5/` - MD5 core
- `srcs/sha256/` - SHA-256 core
- `srcs/whirlpool/` - Whirlpool core and lookup tables

Dispatch uses a function-pointer table, not an `if/else` chain.
