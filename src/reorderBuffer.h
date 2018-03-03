#pragma once

typedef struct ROB_Data
{
	/* uint32_t value; */
	int dest_reg;
	uint8_t ready;
	/* uint32_t exception; */
	/* uint32_t misprediction; */
	/* uint32_t pc; */
	Instruction instr;
}ROB_Data;

typedef struct ROB
{
	ROB_Data* data;
	QueueInfo robInfo;
}ROB;

typedef struct RMT
{
	uint8_t valid;
	int robTag;
}RMT;

extern int ARF_SIZE;

extern RMT* renameMapTable;
extern ROB reorderBuffer;

void Init_RMT_ROB();

int Write_RMT_ROB(Instruction* instr);

int CanWrite_ROB();

void Free_RMT_ROB();