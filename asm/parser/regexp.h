/*
 * regexp.h -- a set of regular expression related functions
 */

#ifndef REGEXP_INCL
#define REGEXP_INCL

int   check_pattern(char*);
void  regfree_all();
void  regexp_test();  // DEBUG

#endif /* REGEXP_INCL */


/*　
Addressing Modes

• R-Type (One or Two Registers)
[op] [func]  [rd] [rs] [sbz]
0000   00    0000 0000 00

 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
+---------------+-------+---------------+---------------+-------+
|      OP       | FUNC  |      RD       |      RS       |  SBZ  |
|     4bits     | 2bits |     4bits     |     4bits     | 2bits |
+---------------+-------+---------------+---------------+-------+


• I-Type (One Register, One Immediate) 
[op] [func]  [rd] [imm]
0100   00    0000 0000 00

 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
+---------------+-------+---------------+-----------------------+
|      OP       | FUNC  |      RD       |       IMMEDIATE       |
|     4bits     | 2bits |     4bits     |        6 bits         |
+---------------+-------+---------------+-----------------------+


• O-Type (Two Registers, One Offset)
[op] [func]  [rd] [rs] [offset]
1000   00    000  000    0000

 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
+---------------+-------+-----------+-----------+---------------+
|      OP       | FUNC  |    RD     |    RS     |    OFFSET     |
|     4bits     | 2bits |   3bits   |   3bits   |    4bits      |
+---------------+-------+-----------+-----------+---------------+


• J-Type (Jump to Target Address)
[op] [func]  [target]
1000   00    0000 0000 00

 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
+---------------+-------+---------------------------------------+
|      OP       | FUNC  |                TARGET                 |
|     4bits     | 2bits |                10 bits                |
+---------------+-------+---------------------------------------+



=======
SUMMARY
=======

00: R-Type (Arithmetic, Logical, Shift)
0000 00 ADD   0x0 0        ADD $rd, $rs    
0000 01 SUB   0x0 1        SUB $rd, $rs    
0000 10 MUL   0x0 2        MUL $rd, $rs    
0000 11 SLT   0x0 3        SLT $rd, $rs    

0001 00 ADDU  0x1 0        ADDU $rd, $rs   
0001 01 SUBU  0x1 1        SUBU $rd, $rs
0001 10 MULU  0x1 2        MULU $rd, $rs
0001 11 SLTU  0x1 3        SLTU $rd, $rs

0010 00 AND   0x2 0        AND $rd, $rs
0010 01 OR    0x2 1        OR  $rd, $rs
0010 10 XOR   0x2 2        XOR $rd, $rs
0010 11 NOR   0x2 3        NOR $rd, $rs

0011 00 SLL   0x3 0        SLL $rd, $rs
0011 01 SRL   0x3 1        SRL $rd, $rs
0011 10 SLA   0x3 2        SLA $rd, $rs
0011 11 ROTL  0x3 3        ROT $rd, $rs 


01: I-Type (Immediate)
0100 00 ADDI  0x4 0        ADDI $rd, imm
0100 01 SUBI  0x4 1        SUBI $rd, imm
0100 10 MULI  0x4 2        MULI $rd, imm
0100 11 SLTI  0x4 3        SLTI $rd, imm

0101 00 ADDIU 0x5 0        ADDIU $rd, imm
0101 01 SUBIU 0x5 1        SUBIU $rd, imm
0101 10 MULIU 0x5 2        MULIU $rd, imm
0101 11 SLTIU 0x5 3        SLTIU $rd, imm

0110 00 ANDI  0x6 0        ANDI $rd, imm
0110 01 ORI   0x6 1        ORI  $rd, imm
0110 10 XORI  0x6 2        XORI $rd, imm
0110 11 NORI  0x6 3        NORI $rd, imm

0111 00 SLLI  0x7 0        SLLI $rd, imm
0111 01 SRLI  0x7 1        SRLI $rd, imm
0111 10 SRAI  0x7 2        SRAI $rd, imm
0111 11 ROTLI 0x7 3        ROTI $rd, imm 


10 : ORJ-Type (Jump/Branch Flow Control)
1010 00 J     0xA 0        J    target   (J-type)
1010 01 JAL   0xA 1        JAL  target   (J-type)
1010 10 JR    0xA 2        JR   $rd      (R-type)
1010 11 JALR  0xA 3        JALR $rd, $rs (R-type)

1011 00 BEQ   0xB 0        BEQ  $rd, $rs, offset (O-type)
1011 01 BNE   0xB 1        BNE  $rd, $rs, offset (O-type)


11 : OR-TYPE (Register/Memory Data Move)
1100 00 LW    0xC 0        LW $rd, $rs, offset (O-type)
1100 01 LB    0xC 1        LB $rd, $rs, offset (O-type)
1100 10 SW    0xC 2        SW $rd, $rs, offset (O-type)
1100 11 SB    0xC 3        SB $rd, $rs, offset (O-type)

1101 00 MFHI  0xD 0        MFHI $rd (R-type)
1101 01 MFLO  0xD 1        MFLO $rd (R-type)
1101 10 MTHI  0xD 2        MTHI $rd (R-type)
1101 11 MTLO  0xD 3        MTLO $rd (R-type)

*/