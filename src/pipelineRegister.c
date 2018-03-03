#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "instructionList.h"
#include "issueQueue.h"
#include "executionList.h"

void Init_Registers()
{
	Init_QueueInfo(&(DE_RN.registerInfo), WIDTH);
	Init_QueueInfo(&(RN_RR.registerInfo), WIDTH);
	Init_QueueInfo(&(RR_DI.registerInfo), WIDTH);
	Init_QueueInfo(&(EX_WB.registerInfo), WIDTH*5);

	DE_RN.data = (Instruction*)malloc(sizeof(Instruction)*WIDTH);
	if (DE_RN.data == NULL)
		_error_exit("malloc");
	RN_RR.data = (Instruction*)malloc(sizeof(Instruction)*WIDTH);
	if (RN_RR.data == NULL)
		_error_exit("malloc");
	RR_DI.data = (RR_Data*)malloc(sizeof(RR_Data)*WIDTH);
	if (RR_DI.data == NULL)
		_error_exit("malloc");
	EX_WB.data = (WB_Data*)malloc(sizeof(WB_Data)*WIDTH*5);
	if (EX_WB.data == NULL)
		_error_exit("malloc");

	memset(DE_RN.data, 0, sizeof(Instruction)*WIDTH);
	memset(RN_RR.data, 0, sizeof(Instruction)*WIDTH);
	memset(RR_DI.data, 0, sizeof(RR_Data)*WIDTH);
	memset(EX_WB.data, 0, sizeof(WB_Data)*WIDTH*5);

	Init_InstructionList();
	Init_IssueQueue();
	Init_ExcutionList();
}

int CanWrite_Register(uint32_t registerName)
{
	switch (registerName)
	{
	case REGISTER_IF_DE:
		return CanWrite_InstructionList();
	case REGISTER_DE_RN:
		return CanWrite_QueueInfo(&(DE_RN.registerInfo), -1);
	case REGISTER_RN_RR:
		return CanWrite_QueueInfo(&(RN_RR.registerInfo), -1);
	case REGISTER_RR_DI:
		return CanWrite_QueueInfo(&(RR_DI.registerInfo), -1);
	case REGISTER_DI_IQ:
		return CanWrite_IssueQueue();
	case REGISTER_IQ_EX:
		return CanWrite_ExcutionList();
	case REGISTER_EX_WB:
		return CanWrite_QueueInfo(&(EX_WB.registerInfo), 0);
	default:
		_error_msg_exit("write invalid register between stages");
	}
}

int Write_Register(uint32_t registerName, void* new_data)
{
	int tail = -1;
	switch (registerName)
	{
	case REGISTER_IF_DE:
		return Write_InstructionList(new_data);
	case REGISTER_DE_RN:
		tail = Write_QueueInfo(&(DE_RN.registerInfo));
		if (tail >= 0)
		{
			Instruction* data = (Instruction*)DE_RN.data;
			data[tail] = *((Instruction*)new_data);
			return EXIT_SUCCESS;
		}
		else
			return EXIT_FAILURE;
	case REGISTER_RN_RR:
		tail = Write_QueueInfo(&(RN_RR.registerInfo));
		if (tail >= 0)
		{
			Instruction* data = (Instruction*)RN_RR.data;
			data[tail] = *((Instruction*)new_data);
			return EXIT_SUCCESS;
		}
		else
			return EXIT_FAILURE;
	case REGISTER_RR_DI:
		tail = Write_QueueInfo(&(RR_DI.registerInfo));
		if (tail >= 0)
		{
			RR_Data* data = (RR_Data*)RR_DI.data;
			data[tail] = *((RR_Data*)new_data);
			return EXIT_SUCCESS;
		}
		else
			return EXIT_FAILURE;
	case REGISTER_DI_IQ:
		return Write_IssueQueue(new_data);
	case REGISTER_IQ_EX:
		return Write_ExcutionList(new_data);
	case REGISTER_EX_WB:
		tail = Write_QueueInfo(&(EX_WB.registerInfo));
		if (tail >= 0)
		{
			WB_Data* data = (WB_Data*)EX_WB.data;
			data[tail] = *((WB_Data*)new_data);
			return EXIT_SUCCESS;
		}
		else
			return EXIT_FAILURE;
	default:
		_error_msg_exit("write invalid register between stages");
	}
}

void* Read_Register(uint32_t registerName)
{
	int head = -1;
	switch (registerName)
	{
	case REGISTER_IF_DE:
		return Read_InstructionList();
	case REGISTER_DE_RN:
		head = Read_QueueInfo(&(DE_RN.registerInfo));
		if (head >= 0)
		{
			Instruction* data = (Instruction*)DE_RN.data;
			return &(data[head]);
		}
		else
			return NULL;
	case REGISTER_RN_RR:
		head = Read_QueueInfo(&(RN_RR.registerInfo));
		if (head >= 0)
		{
			Instruction* data = (Instruction*)RN_RR.data;
			return &(data[head]);
		}
		else
			return NULL;
	case REGISTER_RR_DI:
		head = Read_QueueInfo(&(RR_DI.registerInfo));
		if (head >= 0)
		{
			RR_Data* data = (RR_Data*)RR_DI.data;
			return &(data[head]);
		}
		else
			return NULL;
	case REGISTER_DI_IQ:
		return Read_IssueQueue();
	case REGISTER_IQ_EX:
		return Read_ExecutionList();
	case REGISTER_EX_WB:
		head = Read_QueueInfo(&(EX_WB.registerInfo));
		if (head >= 0)
		{
			WB_Data* data = (WB_Data*)EX_WB.data;
			return &(data[head]);
		}
		else
			return NULL;
	default:
		_error_msg_exit("write invalid register between stages");
	}
}

void Update_Register(uint32_t registerName, int dest_reg)
{
	switch (registerName)
	{
	case REGISTER_IF_DE:
	{
		Execute_InstructionList();
		break;
	}
	case REGISTER_RR_DI:
	{
		if (RR_DI.registerInfo.status != empty)
		{
			int head = RR_DI.registerInfo.head;
			int tail = RR_DI.registerInfo.tail;
			RR_Data* data = RR_DI.data;
			int pointer = head;
			while (RR_DI.registerInfo.status != empty)
			{
				if (data[pointer].src1_tag_val == dest_reg)
					data[pointer].src1_rdy = VALID;
				if (data[pointer].src2_tag_val == dest_reg)
					data[pointer].src2_rdy = VALID;
				pointer = (pointer + 1) % RR_DI.registerInfo.size;
				if (pointer == tail)
					break;
			}
		}
		break;
	}
	case REGISTER_DI_IQ:
	{	
		Update_IssueQueue(dest_reg);
		break; 
	}
	case REGISTER_IQ_EX:
	{
		Execute_ExecutionList();
		break;
	}
	default:
		_error_msg_exit("write invalid register between stages when updating");
	}
}

void Free_Registers()
{
	free(EX_WB.data);
	free(RR_DI.data);
	free(RN_RR.data);
	free(DE_RN.data);
	free(instructionList.data);
	free(issueQueue.data);
	free(executionList);
}

void Print_Process(Instruction instr)
{
	OperationType opt = instr.opt;
	int op = (opt == type_0) ? 0 : (opt == type_1) ? 1 : 2;
	printf("%d ", instr.instrInfo.instrCount - 1);
	printf("fu{%d} ", op);
	printf("src{%d, %d} ", instr.src1_reg, instr.src2_reg);
	printf("dst{%d} ", instr.dest_reg);
	printf("FE{%d, %d} ", instr.instrInfo.IF, instr.instrInfo.DE - instr.instrInfo.IF);
	printf("DE{%d, %d} ", instr.instrInfo.DE, instr.instrInfo.RN - instr.instrInfo.DE);
	printf("RN{%d, %d} ", instr.instrInfo.RN, instr.instrInfo.RR - instr.instrInfo.RN);
	printf("RR{%d, %d} ", instr.instrInfo.RR, instr.instrInfo.DI - instr.instrInfo.RR);
	printf("DI{%d, %d} ", instr.instrInfo.DI, instr.instrInfo.IS - instr.instrInfo.DI);
	printf("IS{%d, %d} ", instr.instrInfo.IS, instr.instrInfo.EX - instr.instrInfo.IS);
	printf("EX{%d, %d} ", instr.instrInfo.EX, instr.instrInfo.WB - instr.instrInfo.EX);
	printf("WB{%d, %d} ", instr.instrInfo.WB, instr.instrInfo.RT - instr.instrInfo.WB);
	printf("RT{%d, %d}\n", instr.instrInfo.RT, instr.instrInfo.FN - instr.instrInfo.RT);
}