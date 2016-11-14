#pragma once

typedef struct ExcutionList
{
	RR_Data instr_data;
	int cycle;
}ExcutionList;

extern ExcutionList* executionList;

void Init_ExcutionList();

int CanWrite_ExcutionList();

int Write_ExcutionList(RR_Data* new_data);

void Execute_ExecutionList();

RR_Data* Read_ExecutionList();