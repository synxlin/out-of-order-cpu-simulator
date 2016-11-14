#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "pipelineRegister.h"
#include "issueQueue.h"

void Init_IssueQueue()
{
	issueQueue.size = IQ_SIZE;
	issueQueue.youngestAge = 0;
	issueQueue.data = (IssueQueue_Data*)malloc(sizeof(IssueQueue_Data)*IQ_SIZE);
	if (issueQueue.data == NULL)
		_error_exit("malloc");
	memset(issueQueue.data, 0, sizeof(IssueQueue_Data)*IQ_SIZE);
}

int CanWrite_IssueQueue()
{
	int i = 0, num_free = 0;
	for (i = 0; i < issueQueue.size; i++)
		if (issueQueue.data[i].valid == INVALID)
			num_free++;
	if (num_free >= WIDTH)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

int Write_IssueQueue(RR_Data* new_data)
{
	int i = 0, k = issueQueue.size;
	for (i = 0; i < issueQueue.size; i++)
		if (issueQueue.data[i].valid == INVALID)
		{
			k = i;
			break;
		}
	if (k == issueQueue.size)
		return EXIT_FAILURE;
	issueQueue.data[k].instr_data = *new_data;
	issueQueue.data[k].valid = VALID;
	issueQueue.youngestAge++;
	issueQueue.data[k].age = issueQueue.youngestAge;
	return EXIT_SUCCESS;
}

RR_Data* Read_IssueQueue()
{
	int i = 0, k = issueQueue.size;
	int oldestAge = issueQueue.youngestAge+1;
	for (i = 0; i < issueQueue.size; i++)
		if (issueQueue.data[i].valid == VALID && issueQueue.data[i].instr_data.src1_rdy == VALID \
			&& issueQueue.data[i].instr_data.src2_rdy == VALID \
			&& issueQueue.data[i].age < oldestAge)
		{
			oldestAge = issueQueue.data[i].age;
			k = i;
		}
	if (k == issueQueue.size)
		return NULL;
	issueQueue.data[k].valid = INVALID;
	return &(issueQueue.data[k].instr_data);
}

void Update_IssueQueue(int src_reg_tag)
{
	int i = 0;
	for (i = 0; i < issueQueue.size; i++)
	{
		if (issueQueue.data[i].valid == VALID)
		{
			/* if src1_tag_val contains value instead of tag, its rdy is already VALID */
			if (issueQueue.data[i].instr_data.src1_tag_val == src_reg_tag)
				issueQueue.data[i].instr_data.src1_rdy = VALID;
			if (issueQueue.data[i].instr_data.src2_tag_val == src_reg_tag)
				issueQueue.data[i].instr_data.src2_rdy = VALID;
		}
	}
}