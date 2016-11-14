#pragma once

#define REGISTER_IF_DE 0
#define REGISTER_DE_RN 1
#define REGISTER_RN_RR 2
#define REGISTER_RR_DI 3
#define REGISTER_DI_IQ 4
#define REGISTER_IQ_EX 5
#define REGISTER_EX_WB 6

typedef struct InstrInfo
{
	int IF;
	int DE;
	int RN;
	int RR;
	int DI;
	int IS;
	int EX;
	int WB;
	int RT;
	int FN;
	int instrCount;
}InstrInfo;

/*
 *	
 *	if reg > 67 means it's renamed
 */
typedef struct Instruction
{
	uint32_t pc;
	OperationType opt;
	int dest_reg;
	int src1_reg;
	int src2_reg;
	InstrInfo instrInfo;
}Instruction;

typedef struct RR_Data
{
	uint32_t pc;
	OperationType opt;
	int dest_reg_tag;
	int src1_tag_val;
	uint8_t src1_rdy;
	int src2_tag_val;
	uint8_t src2_rdy;
	InstrInfo instrInfo;
}RR_Data;

typedef struct WB_Data
{
	/* uint32_t dest_val; */
	int dest_reg;
	InstrInfo instrInfo;
}WB_Data;

typedef struct Register
{
	void* data;
	QueueInfo registerInfo;
}Register;

extern int WIDTH;
extern int ROB_SIZE;

extern Register DE_RN;
extern Register RN_RR;
extern Register RR_DI;
extern Register EX_WB;

typedef struct InstructionArray
{
	Instruction* instructions;
	int size;
}InstructionArray;

void Init_Registers();

int CanWrite_Register(uint32_t registerName);

int Write_Register(uint32_t registerName, void* new_data);

void* Read_Register(uint32_t registerName);

void Update_Register(uint32_t registerName, int dest_reg);

void Free_Registers();

void Print_Process(Instruction instr);