#pragma once

typedef struct InstructionC
{
	Instruction instr_data;
	int cycle;
}InstructionC;

typedef struct InstructionList
{
	InstructionC* data;
	QueueInfo registerInfo;
}InstructionList;

extern InstructionList instructionList;

void Init_InstructionList();

int CanWrite_InstructionList();

int Write_InstructionList(InstructionC* new_data);

void Execute_InstructionList();

Instruction* Read_InstructionList();

void Corret_Instr_IF_CycleTime();