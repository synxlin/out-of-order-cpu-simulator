#pragma once

typedef struct IssueQueue_Data
{
	RR_Data instr_data;
	uint8_t valid;
	int age;
}IssueQueue_Data;

typedef struct IssueQueue
{
	IssueQueue_Data* data;
	int size;
	int youngestAge;
}IssueQueue;

extern int IQ_SIZE;

extern IssueQueue issueQueue;

void Init_IssueQueue();

int CanWrite_IssueQueue();

int Write_IssueQueue(RR_Data* new_data);

RR_Data* Read_IssueQueue();

void Update_IssueQueue(int src_reg_tag);