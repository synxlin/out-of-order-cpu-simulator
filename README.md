Out-of-order CPU Simulator (Dynamic Instruction Scheduling)
=============================================================

(Latest update: 2016/11/30)

## Introduction

This is a out-of-order CPU simulator (dynamic instruction scheduling simulator).

## Usage

Using the command shown as follows.

To complie the program,

>make

To run the program,

> sim_ds < ROB_SIZE \> < IQ_SIZE \> < WIDTH \> < CACHE_SIZE \> < P \> < tracefile \>

### Input

|	Parameters		|	Introduction																					|
|-------------------|:-----------------------------------------------:													|
|	ROB_SIZE:   	|Positive int. This is the number of entries in the ReOrder Buffer (ROB)							|
|	IQ_SIZE:    	|Positive int. This is the number of entries in the Issue Queue (IQ)								|
|	WIDTH:    		|Positive int. This is the superscalar width of all pipeline stages, in terms of the maximum number of instructions in each pipeline stage. The one exception is Writeback: the number of instructions that may complete execution in a given cycle is not limited to WIDTH										|
|	CACHE_SIZE:    	|Non-negative int. This is the instruction cache size in bytes.	0 means no instruction cache		|
|	P:    			|Non-negative int. This is the prefetching flag indicating whether the next‐line prefetching is enabled. 0 means disabled							|
|	tracefile:  	|Character string. Full name of trace file including any extensions									|

### Output

The output of program includes

1. Situaition of every instruction:

> < seq_no \> fu{< op_type \>} src{< src1 \>,< src2 \>} dst{< dst \>} FE{< begin-cycle \>,< duration \>} DE{…} RN{…} RR{…} DI{…} IS{…} EX{…} WB{…} RT{…}

2. some measurements

	a. Total number of instructions in the trace.
	
	b. Total number of cycles to finish the program.
	
	c. Average number of instructions retired per cycle (IPC).
	
	d. Total number of instruction cache hits in the trace.
	
	e. Total number of prefetch hits in the trace.

### Trace File

The simulator reads a trace file in the following format: 

< PC \> < operation type \> < dest reg # \> < src1 reg \# \> < src2 reg \# \>

< PC \> < operation type \> < dest reg # \> < src1 reg \# \> < src2 reg \# \>

...

Where:

o < PC \> is the program counter of the instruction (in hex).

o < operation type \> is either “0”, “1”, or “2”.

o < dest reg # \> is the destination register of the instruction. If it is ‐1, then the instruction does not have a destination register (for example, a conditional branch instruction). Else, it is between 0 and 66.

o < src1 reg # \> is the first source register of the instruction. If it is ‐1, then the instruction does not have a first source register. Else, it is between 0 and 66.

o < src2 reg # \> is the second source register of the instruction. If it is ‐1, then the instruction does not have a second source register. Else, it is between 0 and 66.

For example:

ab120024 0 1 2 3

ab120028 1 4 1 3

ab12002c 2 -1 4 7

Means:

“operation type 0” R1, R2, R3

“operation type 1” R4, R1, R3

“operation type 2” ‐, R4, R7 // no destination register!
