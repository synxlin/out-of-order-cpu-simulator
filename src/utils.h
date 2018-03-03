#pragma once

#define VALID 1
#define INVALID 0

#define _error_exit(fun) { perror(fun); exit(EXIT_FAILURE); }

#define _error_msg_exit(msg) { printf("error: %s", msg); exit(EXIT_FAILURE);}

#define _is_power_of_2(x) (((x) == 0) || ((x) > 0 && !((x) & ((x) - 1))))

typedef enum QueueStatus
{
	empty = 0,
	full = 1,
	available = 2
}QueueStatus;

typedef enum OperationType
{
	type_0 = 1,
	type_1 = 2,
	type_2 = 5
}OperationType;

typedef struct QueueInfo
{
	int head;
	int tail;
	int size;
	QueueStatus status;
}QueueInfo;

typedef struct PipelineStat
{
	int instr_count;
	int cycle;
	double IPC;
	int hit;
	int prefetch_hit;
}PipelineStat;

extern PipelineStat pipelineStat;

extern int IQ_SIZE;

void Init_QueueInfo(QueueInfo* queueInfo, int size);

int CanWrite_QueueInfo(QueueInfo* queueInfo, int type);

int Write_QueueInfo(QueueInfo* queueInfo);

int Read_QueueInfo(QueueInfo* queueInfo);

uint32_t log_2(uint32_t num);

void Print(int argc, char* argv[]);