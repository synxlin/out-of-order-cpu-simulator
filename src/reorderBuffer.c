#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "reorderBuffer.h"

void Init_RMT_ROB()
{
	renameMapTable = (RMT*)malloc(sizeof(RMT)*ARF_SIZE);
	if (renameMapTable == NULL)
		_error_exit("malloc");
	Init_QueueInfo(&(reorderBuffer.robInfo), ROB_SIZE);
	reorderBuffer.data = (ROB_Data*)malloc(sizeof(ROB_Data)*ROB_SIZE);
	if (reorderBuffer.data == NULL)
		_error_exit("malloc");
	memset(reorderBuffer.data, 0, sizeof(ROB_Data)*ROB_SIZE);
	memset(renameMapTable, 0, sizeof(RMT)*ARF_SIZE);
}

int Write_RMT_ROB(Instruction* instr)
{
	int tail = Write_QueueInfo(&(reorderBuffer.robInfo));
	if (tail >= 0)
	{
		reorderBuffer.data[tail].dest_reg = instr->dest_reg;
		/* reorderBuffer.data[tail].pc = instr->pc; */
		reorderBuffer.data[tail].instr = *instr;
		reorderBuffer.data[tail].ready = INVALID;
		if (instr->dest_reg >= 0)
		{
			renameMapTable[instr->dest_reg].robTag = tail + ARF_SIZE;
			renameMapTable[instr->dest_reg].valid = VALID;
		}
		instr->dest_reg = tail + ARF_SIZE;
		return EXIT_SUCCESS;
	}
	else
		return EXIT_FAILURE;
}

int CanWrite_ROB()
{
	return CanWrite_QueueInfo(&(reorderBuffer.robInfo), WIDTH);
}

void Free_RMT_ROB()
{
	free(reorderBuffer.data);
	free(renameMapTable);
}