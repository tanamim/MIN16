Let's design your computer
===

## Introduction

This document provides an overview of the development process from designing a computer processor to build onto a chip via programmable way. Field Programmable Gate Array ([FPGA](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=165&No=502)) is used to actually build a chip. The MIN16 processor is a 16-bit CPU that was built as a term project at Harvard ([CSCI E-93](http://sites.fas.harvard.edu/~cscie287/fall2017/)). In this repository, source code for MIN16 processor, sample assembly program, and useful tools are provided.

## Table of Contents
1. [Building blocks](#building-blocks)
2. [Bit format](#bit-format)
3. [Register file](#register-file)
4. [Instruction Set Architecture](#instruction-set-architecture)
5. [Datapath for instruction](#datapath-for-instruction)
6. [Assembly language](#assembly-language)
7. [Assembler](#assembler)
8. [Emulator](#emulator)
9. [CPU written by VHDL](#cpu-written-by-vhdl)
10. [Compile on FPGA](#compile-on-fpga)
11. [Load memory initialization file](#load-memory-initialization-file)
12. [Run your program](#run-your-program)
13. [Welcome to Github Pages](#welcome-to-github-pages)
14. [Markdown](#markdown)
15. [Jekyll Themes](#jekyll-themes)
16. [Support or Contact](#support-or-contact)

### Building blocks

The development process comes with these 4 building blocks and goes in this order. The first step is to determine **word size** (i.e. how many bits are bound together to carry machine instruction on a digital circuit board). Then you will design bit format to prepare a set of instructions (Assembly language: `AND`, `OR`, `ADD`, `J`, etc). Such design document serves as a reference to assembly programmers. Once assembly language is ready, you need to build an assembler to convert to machine code. Emulator is quite useful to debug your assembler programs before CPU will be working correctly. Finally, CPU is described by hardware description language and should be debugged by your assembly program. You will burn your CPU to an FPGA board, load machine code to the memory, and run the program.

| Directory | Description |
| --------- |------------ |
| doc       | Design documents and presentation materials are stored in this directory. [Instruction Set](./doc/MIN16_Instruction_Set.pdf) defines assembly language and its usage. [Datapath](./doc/MIN16_Datapath_ALL.pdf) defines digital circuit board and visualize how each instruction works on the board. |
| asm       | [Assembler](./asm/parser/parser.c) is a tool to convert an [assembly program](./asm/parser/sample3.txt) into a [machine code](./asm/parser/sample3.mif). |
| emu       | [Emulator](./emu/emulator.c) is a useful debugging tool that can simulate on your computer how assembly program should work on the MIN16 processor |
| cpu       | This directory contains all of the VHDL for the [MIN16 processor](./cpu/min16/min16.vhd), including the [ALU](./cpu/min16/alu.vhd). |

### Bit format
Once word size is determined, how do you divide those bits into parts? Typical arithmetic machine instruction contains three components, Operation Code, Destination Regiser Identifier, and Source Register Identifier.

For example, `ADD $r1, $r10` is an instruction to add the value of source register 10 to destination register 1. This instruction is represented in Hex digit as `0x001a`.
```markdown
[opcode] [rd] [rs]
     ADD  $r1 $r10
00000000 0001 1010

 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
+---------------+--------------+---------------+---------------+
|            OPCODE            |      RD       |      RS       |
|             8bits            |     4bits     |     4bits     |
+---------------+--------------+---------------+---------------+
```

### Register file
How many registers do you need? Above example uses 4bits to identify register, therefore 16 registers are available for computation. The more register number, the more temporary calculation space. But it will limit the number of operation codes. Therefore, you need to find a balance. MIN16 defined 16 registers on page 2 of [instruction set](./doc/MIN16_Instruction_Set.pdf)).

### Instruction Set Architecture
All assembly mnemonics should be defined. In addition to ALU type instructions (`ADD`, `SUB`, `AND`, `OR`, `XOR`, etc), Memory Load and Store type instructions are needed (`LW`, `SW`) so that computer can interact with external input/output device. Also, Jump and Branch instructions (`J`, `JR`, `BEQ`, `BNE`) are needed to implement conditional statement and loop. (See page 5 of [instruction set](./doc/MIN16_Instruction_Set.pdf)).

### Datapath for instruction

### Assembly language

### Assembler

### Emulator

### CPU written by VHDL

### Compile on FPGA

### Load memory initialization file

### Run your program



### Welcome to Github Pages

You can use the [editor on GitHub](https://github.com/tanamim/MIN16/edit/master/README.md) to maintain and preview the content for your website in Markdown files.

Whenever you commit to this repository, GitHub Pages will run [Jekyll](https://jekyllrb.com/) to rebuild the pages in your site, from the content in your Markdown files.

### Markdown

Markdown is a lightweight and easy-to-use syntax for styling your writing. It includes conventions for

```markdown
Syntax highlighted code block

# Header 1
## Header 2
### Header 3

- Bulleted
- List

1. Numbered
2. List

**Bold** and _Italic_ and `Code` text

[Link](url) and ![Image](src)
```

For more details see [GitHub Flavored Markdown](https://guides.github.com/features/mastering-markdown/).

### Jekyll Themes

Your Pages site will use the layout and styles from the Jekyll theme you have selected in your [repository settings](https://github.com/tanamim/MIN16/settings). The name of this theme is saved in the Jekyll `_config.yml` configuration file.

### Support or Contact

Having trouble with Pages? Check out our [documentation](https://help.github.com/categories/github-pages-basics/) or [contact support](https://github.com/contact) and we’ll help you sort it out.
