Let's design your computer
===

## Introduction

This document provides an overview of the development process from designing a computer processor to build onto a chip via programmable way. Field Programmable Gate Array ([FPGA](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=165&No=502)) is used to actually build a chip. The MIN16 processor is a 16-bit CPU that was built as a term project at Harvard ([CSCI E-93](http://sites.fas.harvard.edu/~cscie287/fall2017/)). In this repository, source code for MIN16 processor, sample assembly program, and useful tools are provided.

## Building blocks

The development process comes with these 4 building blocks and goes in this order.

| Directory | Description |
| --------- |------------ |
| doc       | Design documents and presentation materials are stored in this directory. [Instruction Set](./doc/MIN16_Instruction_Set.pdf) defines assembly language and its usage. This is a reference document for assembly programmer. [Datapath](./doc/MIN16_Datapath_ALL.pdf) defines digital circuit board and visualize how each instruction works on the board. |
| asm       | [Assembler](./asm/parser/parser.c) is a tool to convert assembly program into machine language. |
| emu       | [Emulator](./emu/emulator.c) is a useful debugging tool to simulate assembly program on MIN16 processor |
| cpu       | This directory contains all of the VHDL for [MIN16 processor](./cpu/min16/min16.vhd), including the [ALU](./cpu/min16/alu.vhd). |

## Table of Contents
1. [Register file](#register-file)
2. [Bit format](#bit-format)
3. [Instruction Set Architecture](#instruction-set-architecture)
4. [Datapath for instruction](#datapath-for-instruction)
5. [Assembly language](#assembly-language)
6. [Assembler](#assembler)
7. [Emulator](#emulator)
8. [CPU written by VHDL](#cpu-written-by-vhdl)
9. [Compile on FPGA](#compile-on-fpga)
10. [Load memory initialization file](#load-memory-initialization-file)
11. [Run your program](#run-your-program)
12. [Welcome to Github Pages](#welcome-to-github-pages)
13. [Markdown](#markdown)
14. [Jekyll Themes](#jekyll-themes)
15. [Support or Contact](#support-or-contact)

### Register file
How many registers do you need?

### Bit format
How do you divide bits into parts?

### Instruction Set Architecture
All assembly mnemonics should be defined.

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
