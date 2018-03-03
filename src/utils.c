#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "cache.h"

void Init_QueueInfo(QueueInfo* queueInfo, int size)
{
	queueInfo->size = size;
	queueInfo->head = 0;
	queueInfo->tail = 0;
	queueInfo->status = empty;
}

int CanWrite_QueueInfo(QueueInfo* queueInfo, int type)
{
	switch (type)
	{
	case 0:
	{
		if (queueInfo->status == full)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}
	case -1:
	{
		if (queueInfo->status == empty)
			return EXIT_SUCCESS;
		return EXIT_FAILURE;
	}
	default:
	{
		if (queueInfo->status == full)
			return EXIT_FAILURE;
		if (queueInfo->tail >= queueInfo->head)
		{
			if ((queueInfo->size - (queueInfo->tail - queueInfo->head)) >= type)
				return EXIT_SUCCESS;
			else
				return EXIT_FAILURE;
		}
		else
		{
			if ((queueInfo->head - queueInfo->tail) >= type)
				return EXIT_SUCCESS;
			else
				return EXIT_FAILURE;
		}
	}
	}
}

int Write_QueueInfo(QueueInfo* queueInfo)
{
	if (queueInfo->status == full)
		return -1;
	int old_tail = queueInfo->tail;
	queueInfo->tail = (old_tail + 1) % queueInfo->size;
	if (queueInfo->head == queueInfo->tail)
		queueInfo->status = full;
	else
		queueInfo->status = available;
	return old_tail;
}

int Read_QueueInfo(QueueInfo* queueInfo)
{
	if (queueInfo->status == empty)
		return -1;
	int old_head = queueInfo->head;
	queueInfo->head = (old_head + 1) % queueInfo->size;
	if (queueInfo->head == queueInfo->tail)
		queueInfo->status = empty;
	else
		queueInfo->status = available;
	return old_head;
}

/*
*	improved log2 function
*	log2(0) = 0
*/
uint32_t log_2(uint32_t num)
{
	uint32_t result = 0, tmp = num;
	while (tmp >>= 1)
		result++;
	return result;
}

void Print(int argc, char* argv[])
{
	printf("# === Simulator Command =========\n");
	printf("#");
	int j = 0;
	for (j = 0; j < argc; j++)
		printf(" %s", argv[j]);
	printf("\n");
	printf("# === Processor Configuration ===\n");
	printf("# ROB_SIZE 	= %d\n", ROB_SIZE);
	printf("# IQ_SIZE  	= %d\n", IQ_SIZE);
	printf("# WIDTH    	= %d\n", WIDTH);
	printf("# CACHE_SIZE     = %d\n", CACHE_SIZE);
	printf("# PREFETCHING  	 = %d\n", (prefetch_flag == PREFETCH) ? 1 : 0);
	printf("# === Simulation Results ========\n");
	printf("# Dynamic Instruction Count      = %d\n", pipelineStat.instr_count);
	printf("# Cycles                         = %d\n", pipelineStat.cycle);
	printf("# Instructions Per Cycle (IPC)   = %.2f\n", pipelineStat.IPC);
	if (cache != NULL)
		printf("# Instruction Cache Hits:      = %d\n", pipelineStat.hit);
	if (prefetch_flag == PREFETCH)
		printf("# Prefetch Hits:               = %d\n", pipelineStat.prefetch_hit);
}