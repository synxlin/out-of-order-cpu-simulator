#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "reorderBuffer.h"
#include "instructionList.h"
#include "cache.h"
#include "pipeline.h"

void Retire()
{
	int i = 0;
	for (i = 0; i < WIDTH; i++)
	{
		if (reorderBuffer.robInfo.status == empty)
			break;
		int head = reorderBuffer.robInfo.head;
		if (reorderBuffer.data[head].ready == VALID)
		{
			reorderBuffer.data[head].instr.instrInfo.FN = pipelineStat.cycle + 1;
			int dest_reg = reorderBuffer.data[head].dest_reg;
			if (dest_reg >= 0)
				if (renameMapTable[dest_reg].robTag == head + ARF_SIZE)
					renameMapTable[dest_reg].valid = INVALID;

			Print_Process(reorderBuffer.data[head].instr);
			final_size++;
			Read_QueueInfo(&(reorderBuffer.robInfo));
		}
		else
			break;
	}
}

void Writeback()
{
	while (1)
	{
		WB_Data* data = Read_Register(REGISTER_EX_WB);
		if (data == NULL)
			return;
		data->instrInfo.RT = pipelineStat.cycle + 1;
		reorderBuffer.data[data->dest_reg - ARF_SIZE].ready = VALID;
		reorderBuffer.data[data->dest_reg - ARF_SIZE].instr.instrInfo = data->instrInfo;
		Update_Register(REGISTER_RR_DI, data->dest_reg);
	}
	if (CanWrite_Register(REGISTER_EX_WB) == EXIT_FAILURE)
		_error_msg_exit("Writeback stage: failed to read out whole EX/WB register");
}

void Execute()
{
	Update_Register(REGISTER_IQ_EX, 0);
	while (1)
	{
		RR_Data* data = Read_Register(REGISTER_IQ_EX);
		if (data == NULL)
			return;
		data->instrInfo.WB = pipelineStat.cycle + 1;
		WB_Data new_data;
		new_data.dest_reg = data->dest_reg_tag;
		new_data.instrInfo = data->instrInfo;
		if (Write_Register(REGISTER_EX_WB, &new_data) == EXIT_FAILURE)
			_error_msg_exit("more instr executed than writeback capacity");
		int dest_reg = data->dest_reg_tag;
		Update_Register(REGISTER_RR_DI, dest_reg);
		Update_Register(REGISTER_DI_IQ, dest_reg);
	}
	if (CanWrite_Register(REGISTER_IQ_EX) == EXIT_FAILURE)
		_error_msg_exit("Execute stage: failed to read out whole IS/EX register");
}

void Issue()
{
	int i = 0;
	for (i = 0; i < WIDTH; i++)
	{
		if (CanWrite_Register(REGISTER_IQ_EX) == EXIT_SUCCESS)
		{
			RR_Data* data = Read_Register(REGISTER_DI_IQ);
			if (data == NULL)
				return;
			data->instrInfo.EX = pipelineStat.cycle + 1;
			Write_Register(REGISTER_IQ_EX, data);
		}
		else
			return;
	}
}

void Dispatch()
{
	if (CanWrite_Register(REGISTER_DI_IQ) == EXIT_SUCCESS)
	{
		int i = 0;
		for (i = 0; i < WIDTH; i++)
		{
			RR_Data* data = Read_Register(REGISTER_RR_DI);
			if (data == NULL)
				return;
			data->instrInfo.IS = pipelineStat.cycle + 1;
			Write_Register(REGISTER_DI_IQ, data);
		}
		if (CanWrite_Register(REGISTER_RR_DI) == EXIT_FAILURE)
			_error_msg_exit("Dispatch stage: failed to read out whole RR/DI register");
	}
}

void RegRead()
{
	if (CanWrite_Register(REGISTER_RR_DI) == EXIT_SUCCESS)
	{
		int i = 0;
		for (i = 0; i < WIDTH; i++)
		{
			Instruction* data = Read_Register(REGISTER_RN_RR);
			if (data == NULL)
				return;
			RR_Data new_data;
			new_data.pc = data->pc;
			new_data.opt = data->opt;
			new_data.dest_reg_tag = data->dest_reg;
			new_data.src1_tag_val = data->src1_reg;
			new_data.src2_tag_val = data->src2_reg;
			new_data.instrInfo = data->instrInfo;
			if (new_data.src1_tag_val < ARF_SIZE)
				new_data.src1_rdy = VALID;
			else
				new_data.src1_rdy = reorderBuffer.data[new_data.src1_tag_val - ARF_SIZE].ready;
			if (new_data.src2_tag_val < ARF_SIZE)
				new_data.src2_rdy = VALID;
			else
				new_data.src2_rdy = reorderBuffer.data[new_data.src2_tag_val - ARF_SIZE].ready;
			new_data.instrInfo.DI = pipelineStat.cycle + 1;
			Write_Register(REGISTER_RR_DI, &new_data);
		}
		if (CanWrite_Register(REGISTER_RN_RR) == EXIT_FAILURE)
			_error_msg_exit("RegRead stage: failed to read out whole RN/RR register");
	}
	return;
}

void Rename()
{
	if (CanWrite_Register(REGISTER_RN_RR) == EXIT_SUCCESS && CanWrite_ROB() == EXIT_SUCCESS)
	{
		int i = 0;
		for (i = 0; i < WIDTH; i++)
		{
			Instruction* data = Read_Register(REGISTER_DE_RN);
			if (data == NULL)
				return;
			Instruction tmp = *data;
			if (data->src1_reg >= 0)
				if (renameMapTable[data->src1_reg].valid == VALID)
					data->src1_reg = renameMapTable[data->src1_reg].robTag;
			if (data->src2_reg >= 0)
				if (renameMapTable[data->src2_reg].valid == VALID)
					data->src2_reg = renameMapTable[data->src2_reg].robTag;
			Write_RMT_ROB(&tmp);
			data->dest_reg = tmp.dest_reg;
			data->instrInfo.RR = pipelineStat.cycle + 1;
			Write_Register(REGISTER_RN_RR, data);
		}
		if (CanWrite_Register(REGISTER_DE_RN) == EXIT_FAILURE)
			_error_msg_exit("Rename stage: failed to read out whole DE/RN register");
	}
	return;
}

void Decode()
{
	if (CanWrite_Register(REGISTER_DE_RN) == EXIT_SUCCESS)
	{
		int i = 0;
		for (i = 0; i < WIDTH; i++)
		{
			Instruction* data = Read_Register(REGISTER_IF_DE);
			if (data == NULL)
				return;
			data->instrInfo.RN = pipelineStat.cycle + 1;
			Write_Register(REGISTER_DE_RN, data);
		}
		if (CanWrite_Register(REGISTER_IF_DE) == EXIT_FAILURE)
			_error_msg_exit("Decode stage: failed to read out whole IF/DE register");
	}
}

void Fetch()
{
	Update_Register(REGISTER_IF_DE, 0);
	Cache_Update();
	if (CanWrite_Register(REGISTER_IF_DE) == EXIT_SUCCESS)
	{
		int i = 0;
		for (i = 0; i < WIDTH; i++)
		{
			InstructionC instr;
			int op;
			uint8_t line;
			int result = fscanf(trace_file_fp, "%x %d %d %d %d%c", &instr.instr_data.pc, \
				&op, &instr.instr_data.dest_reg, &instr.instr_data.src1_reg, \
				&instr.instr_data.src2_reg, &line);
			if (result == EOF)
			{
				trace_over = VALID;
				break;
			}
			switch (op)
			{
			case 0:
				instr.instr_data.opt = type_0;
				break;
			case 1:
				instr.instr_data.opt = type_1;
				break;
			case 2:
				instr.instr_data.opt = type_2;
			default:
				break;
			}
			Block instr_block = Read(instr.instr_data.pc);
			instr.cycle = instr_block.cycle;

			if (instr_block.cycle == 0)
				pipelineStat.hit++;
			if (instr_block.prefetch_Bit == PREFETCH && instr_block.cycle == 0)
				pipelineStat.prefetch_hit++;

			pipelineStat.instr_count++;
			instr.instr_data.instrInfo.instrCount = pipelineStat.instr_count;
			instr.instr_data.instrInfo.IF = pipelineStat.cycle;
			instr.instr_data.instrInfo.DE = pipelineStat.cycle + 1;

			Write_Register(REGISTER_IF_DE, &instr);
		}
		Corret_Instr_IF_CycleTime();
	}
}

uint8_t advance_cycle()
{
	pipelineStat.cycle++;
	if (trace_over == VALID)
		if (final_size == pipelineStat.instr_count)
		{
			pipelineStat.IPC = ((double)pipelineStat.instr_count)/((double)pipelineStat.cycle);
			return INVALID;
		}
	return VALID;
}