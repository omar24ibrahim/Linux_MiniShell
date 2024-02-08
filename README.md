# Linux-Shell
This project is implementing a Linux shell, handling execution of commands using functions as: fork(), execvp(), pipe(), and wait().
The user's input is taken by a flex tool to be identified then parsed by a yacc parser.
## Table of Contents
- [Linux-Shell](#linux-shell)
  - [Features](#features)
  - [How to run](#how-to-run)
  - [Lexical Analysis](#lexical-analysis)
  - [Yacc Parser](#yacc-parser)

## Features
- The shell supports the following features:
    * Running commands in the foreground and background
    * Redirection of input, output, and error
    * Piping of commands
    * Changing the current working directory
    * Log file for all commands run in the shell
    * Non-terminating shell by Ctrl+C
    * Exit command to terminat the shell

## How to run
1. install the flex, bison packages, gcc and make
```bash
$ sudo apt-get install flex bison gcc make
```
2. Compile the shell by running the command `make`
```bash
$ make
```
3. Run the shell using the command `./shell`

## Lexical Analysis
- The lexical analysis is done using the flex tool. The file `shell.l` contains the regular expression for the shell.
- The regular expression is used to generate the file `lex.yy.c` which contains the lexical analyzer. 
- The lexical analyzer is used to tokenize the input from the user. The tokens are then used to parse the input (by yacc parser).


## Yacc Parser
- The yacc parser is used to generate a parser from a context-free grammar. The file `shell.y` contains the context-free grammar for the shell. 
- The context-free grammar is used to generate the file `y.tab.c` which contains the parser.
- The parser is used to parse the input from the user and generate a parse tree. The parse tree is then used to execute the commands.
