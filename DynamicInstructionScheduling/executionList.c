#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "executionList.h"

void Init_ExcutionList()
{
	executionList = (ExcutionList*)malloc(sizeof(ExcutionList)*WIDTH*5);
	if (executionList == NULL)
		_error_exit("malloc");
	memset(executionList, 0, sizeof(ExcutionList)*WIDTH * 5);
	int i = 0;
	for (i = 0; i < WIDTH * 5; i++)
		executionList[i].cycle = -1;
}

int CanWrite_ExcutionList()
{
	int i = 0;
	for (i = 0; i < WIDTH * 5; i++)
		if (executionList[i].cycle == -1)
			return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

int Write_ExcutionList(RR_Data* new_data)
{
	int i = 0, k = WIDTH * 5;
	for (i = 0; i < WIDTH*5; i++)
		if (executionList[i].cycle == -1)
		{
			k = i;
			break;
		}
	if (k == WIDTH * 5)
		return EXIT_FAILURE;
	executionList[k].instr_data = *new_data;
	executionList[k].cycle = new_data->opt;
	return EXIT_SUCCESS;
}

void Execute_ExecutionList()
{
	int i = 0;
	for (i = 0; i < WIDTH * 5; i++)
		if (executionList[i].cycle != -1)
			executionList[i].cycle -= 1;
}

RR_Data* Read_ExecutionList()
{
	int i = 0, k = WIDTH * 5;
	for (i = 0; i < WIDTH * 5; i++)
		if (executionList[i].cycle == 0)
		{
			k = i;
			break;
		}
	if (k == WIDTH * 5)
		return NULL;
	executionList[k].cycle = -1;
	return &(executionList[k].instr_data);
}