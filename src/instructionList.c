#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "instructionList.h"

void Init_InstructionList()
{
	Init_QueueInfo(&(instructionList.registerInfo), WIDTH);
	instructionList.data = (InstructionC*)malloc(sizeof(InstructionC)*WIDTH);
	if (instructionList.data == NULL)
		_error_exit("malloc");
	memset(instructionList.data, 0, sizeof(InstructionC)*WIDTH);
	int i = 0;
	for (i = 0; i < WIDTH; i++)
		instructionList.data[i].cycle = -1;
}

int CanWrite_InstructionList()
{
	return CanWrite_QueueInfo(&(instructionList.registerInfo), -1);
}

int Write_InstructionList(InstructionC* new_data)
{
	int tail = Write_QueueInfo(&(instructionList.registerInfo));
	if (tail >= 0)
	{
		instructionList.data[tail] = *new_data;
		return EXIT_SUCCESS;
	}
	else
		return EXIT_FAILURE;
}

void Execute_InstructionList()
{
	int i = 0;
	for (i = 0; i < WIDTH; i++)
		if (instructionList.data[i].cycle > 0)
			instructionList.data[i].cycle -= 1;
}

Instruction* Read_InstructionList()
{
	if (instructionList.registerInfo.status == empty)
		return NULL;
	int head = instructionList.registerInfo.head;
	int tail = instructionList.registerInfo.tail;
	int pointer = head;
	while (1)
	{
		if (instructionList.data[pointer].cycle != 0)
			return NULL;
		pointer = (pointer + 1) % instructionList.registerInfo.size;
		if (pointer == tail)
			break;
	}
	head = Read_QueueInfo(&(instructionList.registerInfo));
	return &(instructionList.data[head].instr_data);
}

void Corret_Instr_IF_CycleTime()
{
	if (instructionList.registerInfo.status == empty)
		return;
	int head = instructionList.registerInfo.head;
	int tail = instructionList.registerInfo.tail;
	int pointer = head;
	int maxCycle = 0;
	while (1)
	{
		if (instructionList.data[pointer].cycle > maxCycle)
			maxCycle = instructionList.data[pointer].cycle;
		pointer = (pointer + 1) % instructionList.registerInfo.size;
		if (pointer == tail)
			break;
	}
	pointer = head;
	while (1)
	{
		instructionList.data[pointer].instr_data.instrInfo.IF += maxCycle;
		instructionList.data[pointer].instr_data.instrInfo.DE += maxCycle;
		pointer = (pointer + 1) % instructionList.registerInfo.size;
		if (pointer == tail)
			break;
	}
	return;
}