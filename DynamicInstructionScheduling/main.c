#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "reorderBuffer.h"
#include "instructionList.h"
#include "issueQueue.h"
#include "executionList.h"
#include "cache.h"
#include "pipeline.h"

int WIDTH;
int ROB_SIZE;
int IQ_SIZE;
int ARF_SIZE;
uint32_t CACHE_SIZE;
uint32_t blockSize;
uint8_t prefetch_flag;

InstructionList instructionList;
Register DE_RN;
Register RN_RR;
Register RR_DI;
IssueQueue issueQueue;
ExcutionList* executionList;
Register EX_WB;
ROB reorderBuffer;
RMT* renameMapTable;
Cache* cache;

PipelineStat pipelineStat;
int final_size;

FILE* trace_file_fp;
uint8_t trace_over;
#ifdef DBG
FILE *debug_fp;
#endif

int main(int argc, char* argv[])
{
	if (argc != 7)
		_error_msg_exit("wrong number of parameters");
	ROB_SIZE = atoi(argv[1]);
	IQ_SIZE = atoi(argv[2]);
	WIDTH = atoi(argv[3]);
	CACHE_SIZE = atoi(argv[4]);
	prefetch_flag = atoi(argv[5]);
	prefetch_flag = (prefetch_flag == 0) ? NON_PREFETCH : PREFETCH;
	char* trace_file = argv[6];
	trace_file_fp = fopen(trace_file, "r");
	if (trace_file_fp == NULL)
		_error_exit("fopen");
	trace_over = INVALID;
	ARF_SIZE = 67;
	blockSize = 64;
	uint32_t assoc = 4;

	Init_Cache(assoc);
	Init_Registers();
	Init_RMT_ROB();
	memset(&pipelineStat, 0, sizeof(PipelineStat));
	final_size = 0;

#ifdef DBG
	debug_fp = fopen("debug.txt", "w");
	if (debug_fp == NULL)
		_error_exit("fopen");
#endif

	do
	{
		Retire();

		Writeback();

		Execute();

		Issue();

		Dispatch();

		RegRead();

		Rename();

		Decode();

		Fetch();
	} while (advance_cycle() == VALID);

	Print(argc, argv);

	Free_Cache();
	Free_Registers();
	Free_RMT_ROB();
	return EXIT_SUCCESS;
}