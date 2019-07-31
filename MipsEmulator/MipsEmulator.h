#pragma once
#define $zero 0;
#define $at registers[1]
#define $v0 registers[2]
#define $v1 registers[3]
#define $a0 registers[4]
#define $a1 registers[5]
#define $a2 registers[6]
#define $a3 registers[7]
#define $t0 registers[8]
#define $t1 registers[9]
#define $t2 registers[10]
#define $t3 registers[11]
#define $t4 registers[12]
#define $t5 registers[13]
#define $t6 registers[14]
#define $t7 registers[15]
#define $s0 registers[16]
#define $s1 registers[17]
#define $s2 registers[18]
#define $s3 registers[19]
#define $s4 registers[20]
#define $s5 registers[21]
#define $s6 registers[22]
#define $s7 registers[23]
#define $t8 registers[24]
#define $t9 registers[25]
#define $k0 registers[26]
#define $k1 registers[27]
#define $gp registers[28]
#define $sp registers[29]
#define $fp registers[30]
#define $ra registers[31]
void decodeAndExecute(int);

void add(int, int, int);
void addu(int, int, int);
void _and(int, int, int);
void jalr(int);
void jr(int);
void nor(int, int, int);
void _or(int, int, int);
void slt(int, int, int);
void sltu(int, int, int);
void sll(int, int, int);
void sllv(int, int, int);
void sra(int, int, int);
void srl(int, int, int);
void srlv(int, int, int);
void sub(int, int, int);
void subu(int, int, int);
void _xor(int, int, int);
void mul(int, int, int);
void j(int);
void jal(int);
void addi(int, int, int);
void addiu(int, int, int);
void andi(int, int, int);
void beq(int, int, int);
void bne(int, int, int);
void lb(int, int);
void lbu(int, int);
void lui(int, int);
void lw(int, int);
void ori(int, int, int);
void slti(int, int, int);
void sltiu(int, int, int);
void sb(int, int);
void sw(int, int);
void xori(int, int, int);
int toJTA(int);
int toBTA(int);
void printRegisterSummary();
void syscall();
void loadProgram();
int getInstructionAtProgramCounter();
int signext(int, int);