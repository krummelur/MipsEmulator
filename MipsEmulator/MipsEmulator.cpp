// MipsEmulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdio.h>
#include "MipsEmulator.h"
int registers[32];
int programCounter = 0;

int main()
{
	printRegisterSummary();
	decodeAndExecute(554172424);
	decodeAndExecute(17319968);
	decodeAndExecute(1898467330);
	printRegisterSummary();
}



// Decodes a 32 bit MIPS instruction and executes the appropriate function
void decodeAndExecute(int instruction) {
	programCounter += 4;
	printf("decoding and executing instruction: (0x%08x)\n", instruction);
	registers[0] = 0;
	char errorMessageBuffer[1024];
	int op = instruction >> 26 & 0x3f;
	printf("op: (0x%08x)\n", op);
	int rs = instruction >> 21 & 0x1f;
	printf("rs: (0x%08x)\n", rs);
	int rt = instruction >> 16 & 0x1f;
	printf("rt: (0x%08x)\n", rt);
	int rd = instruction >> 11 & 0x1f;
	printf("rd: (0x%08x)\n", rd);
	int shamt = instruction >> 6 & 0x1f;
	printf("shamt: (0x%08x)\n", shamt);
	int imm = instruction & 0xffff;
	printf("imm: (0x%08x)\n", imm);
	int addr = instruction & 0x03ffffff;
	printf("addr: (0x%08x)\n", addr);
	int funct = instruction & 0x3f;
	printf("funct: (0x%08x)", funct);

	//opcode == 0 -> it is an R-type instruction
	if (!op || op == 28) {
		switch (funct) {
			case 32: add(rd, rs, rt);
				break;
			case 33: addu(rd, rs, rt);
				break;
			case 36: _and(rd, rs, rt);
				break;
			case 9: jalr(rs);
				break;
			case 8: jr(rs);
				break;
			case 39: nor(rd, rs, rt);
				break;
			case 37: _or(rd, rs, rt);
				break;
			case 42: slt(rd, rs, rt);
				break;
			case 43: sltu(rd, rs, rt);
				break;
			case 0: sll(rd, rt, shamt);
				break;
			case 4: sllv(rd, rt, rs);
				break;
			case 3: sra(rd, rt, shamt);
				break;
			case 2:
				if(op == 0)
					srl(rd, rt, shamt);
				else 
					mul(rd, rs, rt);
				break;
			case 6: srlv(rd, rt, rs);
				break;
			case 34: sub(rd, rs, rt);
				break;
			case 35: subu(rd, rs, rt);
				break;
			case 38: _xor(rd, rs, rt);
				break;
		default: 
			sprintf_s(errorMessageBuffer, 1024, "The instruction was not a valid mips instruction (0x%08x)\n", instruction);
			throw std::invalid_argument(errorMessageBuffer);
		}
		return;
	}
	if (op == 2) {
		j(addr);
		return;
	}
	if (op == 3) {
		jal(addr);
		return;
	}
	switch (op) {
		case 8: addi(rt, rs, imm);
			break;
		case 9: addiu(rt, rs, imm);
			break;
		case 12: andi(rt, rs, imm);
			break;
		case 4: beq(rs, rt, imm);
			break;
		case 5: bne(rs, rt, imm);
			break;
		case 32: lb(rt, rs + imm);
			break;
		case 36: lbu(rt, rs + imm);
			break;
		case 15: lui(rt, imm);
			break;
		case 35: lw(rt, rs + imm);
			break;
		case 13: ori(rt, rs, imm);
			break;
		case 10: slti(rt, rs, imm);
			break;
		case 11: sltiu(rt, rs, imm);
			break;
		case 40: sb(rt, rs + imm);
			break;
		case 43: sw(rt, rs + imm);
			break;
		case 14: xori(rt, rs, imm);
			break;
		default: 
			sprintf_s(errorMessageBuffer, 1024, "The instruction was not a valid mips instruction (0x%08x)\n", instruction);
			throw std::invalid_argument(errorMessageBuffer);
	}
}

void addi(int rt, int rs, int imm) {
	registers[rt] = registers[rs] + signext(imm, 16);
}

void addiu(int rt, int rs, int imm) {
	registers[rt] = (unsigned int)registers[rs] + (unsigned int)signext(imm, 16);
}

void andi(int rt, int rs, int imm) {
	registers[rt] = registers[rs] & imm;
}

void beq(int rt, int rs, int imm) {
	if (registers[rt] == registers[rs])
		programCounter = toBTA(imm);
}

void bne(int rt, int rs, int imm) {
	if (registers[rt] != registers[rs])
		programCounter = toBTA(imm);
}

void lb(int rt, int addr) {}

void lbu(int rt, int addr) {}

void lui(int rt, int imm) {
	registers[rt] = imm << 16;
}

void lw(int rt, int addr) {}

void ori(int rt, int rs, int imm) {
	registers[rt] = registers[rs] | imm;
}

void slti(int rt, int rs, int imm) {
	registers[rt] = registers[rs] < signext(imm, 16);
}

void sltiu(int rt, int rs, int imm) {
	registers[rt] = (unsigned int)registers[rs] < (unsigned int)signext(imm, 16);
}

void sb(int rt, int imm) {}

void sw(int rt, int imm) {}

void xori(int rt, int rs, int imm) {
	registers[rt] = registers[rs] ^ signext(imm, 16);
}
// Sign extends and adds the values from 2 registers and puts it into a third register
void add(int rd, int rs, int rt) {
	registers[rd] = registers[signext(rs,6)] + registers[signext(rt,6)];
}

// adds the values without signextension from 2 registers and puts result into third register
void addu(int rd, int rs, int rt) {
	registers[rd] = registers[rs] + registers[rt];
}

void _and(int rd, int rs, int rt) {
	registers[rd] = registers[rs] & registers[rt];
}

void jalr(int rs) {
	$ra = programCounter;
	programCounter += rs;
}

void jr(int rs) {
	programCounter = registers[rs];
}

void nor(int rd, int rs, int rt) {
	registers[rd] = ~(registers[rt] | registers[rs]);
}
void _or(int rd, int rs, int rt) {
	registers[rd] = registers[rt] | registers[rs];
}

void slt(int rd, int rs, int rt) {
	registers[rd] = registers[rs] < registers[rt] ? 1 : 0;
}

void sltu(int rd, int rs, int rt) {
	registers[rd] = (unsigned int)registers[rs] < (unsigned int)registers[rt] ? 1 : 0;
}

void sll(int rd, int rt, int shamt) {
	registers[rd] = registers[rt] << shamt;
}

void sllv(int rd, int rs, int rt) {
	registers[rd] = registers[rt] << registers[rs & 0x1f];
}

void sra(int rd, int rt, int shamt) {
	//Make sure that this will work on all compilers by setting the shifted bits to 1 / 0
	int fillValue = registers[rt] >> 0x1f;
	registers[rd] = (registers[rd] >> shamt);
	registers[rd] = fillValue ? registers[rd] | (~0x0 << 32 - shamt) : registers[rd] & ~(~0x0 << 32 - shamt);
}

void srl(int rd, int rt, int shamt) {
	//Make sure that this will work on all compilers by always setting left bits to 0
	registers[rd] = (registers[rd] >> shamt) & ~(~0x0 << (32 - shamt));
}

void srlv(int rd, int rs, int rt) {
	registers[rd] = (registers[rd] >> (registers[rt] & 0x1f)) & ~(~0x0 << (32 - (registers[rt] & 0x1f)));
}


int signext(int num, int len) {
	if (num >> len) {
		num = num | 0xffc0;
	}
	return num;
}

void sub(int rd, int rs, int rt) {
	registers[rd] = registers[rs] - registers[rt];
}

void subu(int rd, int rs, int rt) {
	registers[rd] = (unsigned int)registers[rs] - (unsigned int)registers[rt];
}

void _xor(int rd, int rs, int rt) {
	registers[rd] = registers[rs] ^ registers[rt];
}

void mul(int rd, int rs, int rt) {
	registers[rd] = registers[rs] * registers[rt];
}

void j(int address) {
	programCounter = toJTA(address);
}

void jal(int address) {
	$ra = programCounter;
	programCounter = toJTA(address);
}



int toJTA(int address) {
	return programCounter = programCounter & 0xf0000 | (address << 2 & 0x0fffc);
}

int toBTA(int offset) {
	return programCounter + signext(offset, 16) * 4;
}

void printRegisterSummary() {
	printf("$at: (0x%08x)\n", $at);
	printf("$v0: (0x%08x)\n", $v0);
	printf("$v1: (0x%08x)\n", $v1);
	printf("$a0: (0x%08x)\n", $a0);
	printf("$a1: (0x%08x)\n", $a1);
	printf("$a2: (0x%08x)\n", $a2);
	printf("$a3: (0x%08x)\n", $a3);
	printf("$t0: (0x%08x)\n", $t0);
	printf("$t1: (0x%08x)\n", $t1);
	printf("$t2: (0x%08x)\n", $t2);
	printf("$t3: (0x%08x)\n", $t3);
	printf("$t4: (0x%08x)\n", $t4);
	printf("$t5: (0x%08x)\n", $t5);
	printf("$t6: (0x%08x)\n", $t6);
	printf("$t7: (0x%08x)\n", $t7);
	printf("$s0: (0x%08x)\n", $s0);
	printf("$s1: (0x%08x)\n", $s1);
	printf("$s2: (0x%08x)\n", $s2);
	printf("$s3: (0x%08x)\n", $s3);
	printf("$s4: (0x%08x)\n", $s4);
	printf("$s5: (0x%08x)\n", $s5);
	printf("$s6: (0x%08x)\n", $s6);
	printf("$s7: (0x%08x)\n", $s7);
	printf("$t8: (0x%08x)\n", $t8);
	printf("$t9: (0x%08x)\n", $t9);
	printf("$k0: (0x%08x)\n", $k0);
	printf("$t0: (0x%08x)\n", $t0);
	printf("$k1: (0x%08x)\n", $k1);
	printf("$gp: (0x%08x)\n", $gp);
	printf("$sp: (0x%08x)\n", $sp);
	printf("$fp: (0x%08x)\n", $fp);
	printf("$ra: (0x%08x)\n", $ra);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
