// MipsEmulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm> 
#include "MipsEmulator.h"
#define _debug 0
int registers[32];
char* programMemory = (char*) malloc(totalMemSz);
size_t programLen;
size_t dataLen;
int programSectionSz = 4096;
int dataSectionSz = 4096;
int stackSectionSz = 512;
int progMemOffs = 0x00400000;
int dataMemOffs = 0x10010000;
int stackMemOffs = 0x7fffeffc;
int programCounter = progMemOffs;

int main()
{
	loadProgram();
	$sp = 0x7fffeffc;
	while(programCounter < progMemOffs + programLen)
		decodeAndExecute(getInstructionAtProgramCounter());
	printRegisterSummary();
}

int virtualAddrToMemIndex(int address) {
	//is a stack address
	if (address > stackMemOffs - stackSectionSz && address <= stackMemOffs) {
		return totalMemSz - 1 - stackMemOffs - address;
	}
	//is data memory
	if (address >= dataMemOffs && address < dataMemOffs + dataSectionSz) {
		return address - dataMemOffs;
	}

	//is program memory
	if (address >= progMemOffs && address < progMemOffs + programSectionSz) {
		return dataSectionSz + address - progMemOffs;
	}

	char errorMessageBuffer[1024];
	sprintf_s(errorMessageBuffer, 1024, "Trying to address invalid memory segment: (0x%08x)\n", address);
	throw std::invalid_argument(errorMessageBuffer);
}

int getInstructionAtProgramCounter() {
	int firstByte = programCounter - progMemOffs + dataSectionSz;
	return int((unsigned char)(programMemory[firstByte]) << 24 |
		(unsigned char)(programMemory[firstByte + 1]) << 16 |
		(unsigned char)(programMemory[firstByte + 2]) << 8 |
		(unsigned char)(programMemory[firstByte + 3]));
}

char* integerToCharArray(int number) {
	char result[4];
	result[3] = number & 0xff;
	result[2] = number >> 4 & 0xff;
	result[1] = number >> 8 & 0xff;
	result[0] = number >> 12 & 0xff;
	return result;
}

void loadProgram() {
	const char* programString = (char*) ".text";
	const char* dataString = (char*) ".data";
	std::ifstream infile("../print-string-from-data.mips");

	infile.seekg(0, infile.end);
	int inputDataLen = infile.tellg();
	char* inputFileData = (char*) malloc(inputDataLen);
	infile.seekg(0, infile.beg);
	infile.read(inputFileData, inputDataLen);

	char* dataStartAddr = std::search(inputFileData, inputFileData + inputDataLen, dataString, dataString + 5);
	char* programStartAddr = std::search(inputFileData, inputFileData + inputDataLen, programString, programString + 5);

	if (programStartAddr == inputFileData || dataStartAddr > programStartAddr) {
		throw std::invalid_argument("The file segment were missing or in wrong order, data segment must come before program segment");
	}

	programLen = inputDataLen - (programStartAddr - dataStartAddr) - 5;
	dataLen = programStartAddr - dataStartAddr - 5;
	inPlaceLittleEndianToBigEndian(dataStartAddr + 5, dataLen / 4);
	std::copy(dataStartAddr + 5, programStartAddr, programMemory);
	std::copy(programStartAddr + 5, (inputFileData + inputDataLen), programMemory + dataSectionSz);

	free(inputFileData);
	infile.close();
}

// Decodes a 32 bit MIPS instruction and executes the appropriate function
void decodeAndExecute(int instruction) {
	programCounter += 4;

	printf("executing instruction at addr: 0x%08x (0x%08x)\n", programCounter, instruction);


	registers[0] = 0;
	char errorMessageBuffer[1024];
	int op = instruction >> 26 & 0x3f;
	int rs = instruction >> 21 & 0x1f;
	int rt = instruction >> 16 & 0x1f;
	int rd = instruction >> 11 & 0x1f;
	int shamt = instruction >> 6 & 0x1f;
	int imm = instruction & 0xffff;
	int addr = instruction & 0x03ffffff;
	int funct = instruction & 0x3f;

#ifdef debug
	printf("op: (0x%08x)\n", op);
	printf("rs: (0x%08x)\n", rs);
	printf("rt: (0x%08x)\n", rt);
	printf("rd: (0x%08x)\n", rd);
	printf("shamt: (0x%08x)\n", shamt);
	printf("imm: (0x%08x)\n", imm);
	printf("addr: (0x%08x)\n", addr);
	printf("funct: (0x%08x)", funct);
#endif

	if (instruction == 0x00000000c) {
		syscall();
		return;
	}

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
		case 32: lb(rt, rs, imm);
			break;
		case 36: lbu(rt, rs, imm);
			break;
		case 15: lui(rt, imm);
			break;
		case 35: lw(rt, rs, imm);
			break;
		case 13: ori(rt, rs, imm);
			break;
		case 10: slti(rt, rs, imm);
			break;
		case 11: sltiu(rt, rs, imm);
			break;
		case 40: sb(rt, rs, imm);
			break;
		case 43: sw(rt, rs, imm);
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

void lb(int rt, int rs, int imm) {
	registers[rt] = signext((int)programMemory[virtualAddrToMemIndex(registers[rs]+ signext(imm,8))],8);
}

void lbu(int rt, int rs, int imm) { 
	registers[rt] = (int)programMemory[virtualAddrToMemIndex(registers[rs] + signext(imm,8))];
}

void lui(int rt, int imm) {
	registers[rt] = imm << 16;
}

void lw(int rt, int rs, int imm) {
	registers[rt] = programMemory[virtualAddrToMemIndex(registers[rs] + signext(imm, 8))];
}

void ori(int rt, int rs, int imm) {
	registers[rt] = registers[rs] | imm;
}

void slti(int rt, int rs, int imm) {
	registers[rt] = registers[rs] < signext(imm, 16);
}

void sltiu(int rt, int rs, int imm) {
	registers[rt] = (unsigned int)registers[rs] < (unsigned int)signext(imm, 16);
}

void sb(int rt, int rs, int imm) {
	int byteToStore = integerToCharArray(registers[rt])[0];
	programMemory[virtualAddrToMemIndex(registers[rs] + signext(imm,8))] = byteToStore;
}

void sw(int rt, int rs, int imm) {
	char* registerAsCharArray = integerToCharArray(registers[rt]);
	int i;
	for (i = 0; i < 4; i++) {
	programMemory[virtualAddrToMemIndex(registers[rs] + signext(imm,8))+i] = registerAsCharArray[i];
	}
}

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
	int test = num >> (len-1);
	if (num >> (len-1)) {
		num = num | ~0x0 << len;
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
	int test = signext(offset, 16);
	return programCounter + signext(offset, 16) * 4;
}

//Some syscalls are implemented for reading and priting strings and integers. 
//All syscalls utilizing registers f0-f31 are left out.
void syscall() {
	switch ($v0) {
	case 1: printf("%d\n", $a0);
		break;
	case 4: printf("%s\n", (char*) &programMemory[virtualAddrToMemIndex($a0)]);
		break;
	case 5: std::cin >> $v0;
		break;
	case 11: printf("%c\n", $a0);
	}
}

void inPlaceLittleEndianToBigEndian(char* data, int numWords) {
	int i;
	for (i = 0; i < numWords; i++) {
		int byteIndex = i * 4;
		char byte1 = *(data + byteIndex);
		char byte2 = *(data + byteIndex + 1);
		*(data + byteIndex) = *(data + byteIndex + 3);
		*(data + byteIndex + 1) = *(data + byteIndex + 2);
		*(data + byteIndex + 2) = byte2;
		*(data + byteIndex + 3) = byte1;
	}
}

void printRegisterSummary() {
	printf("\n\n");
	printf("$at: 0x%08x\n", $at);
	printf("$v0: 0x%08x\n", $v0);
	printf("$v1: 0x%08x\n", $v1);
	printf("$a0: 0x%08x\n", $a0);
	printf("$a1: 0x%08x\n", $a1);
	printf("$a2: 0x%08x\n", $a2);
	printf("$a3: 0x%08x\n", $a3);
	printf("$t0: 0x%08x\n", $t0);
	printf("$t1: 0x%08x\n", $t1);
	printf("$t2: 0x%08x\n", $t2);
	printf("$t3: 0x%08x\n", $t3);
	printf("$t4: 0x%08x\n", $t4);
	printf("$t5: 0x%08x\n", $t5);
	printf("$t6: 0x%08x\n", $t6);
	printf("$t7: 0x%08x\n", $t7);
	printf("$s0: 0x%08x\n", $s0);
	printf("$s1: 0x%08x\n", $s1);
	printf("$s2: 0x%08x\n", $s2);
	printf("$s3: 0x%08x\n", $s3);
	printf("$s4: 0x%08x\n", $s4);
	printf("$s5: 0x%08x\n", $s5);
	printf("$s6: 0x%08x\n", $s6);
	printf("$s7: 0x%08x\n", $s7);
	printf("$t8: 0x%08x\n", $t8);
	printf("$t9: 0x%08x\n", $t9);
	printf("$k0: 0x%08x\n", $k0);
	printf("$t0: 0x%08x\n", $t0);
	printf("$k1: 0x%08x\n", $k1);
	printf("$gp: 0x%08x\n", $gp);
	printf("$sp: 0x%08x\n", $sp);
	printf("$fp: 0x%08x\n", $fp);
	printf("$ra: 0x%08x\n", $ra);

}