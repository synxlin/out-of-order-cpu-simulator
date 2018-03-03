#pragma once

extern Cache* cache;

extern PipelineStat pipelineStat;
extern FILE* trace_file_fp;
extern uint8_t trace_over;
extern int final_size;

void Retire();

void Writeback();

void Execute();

void Issue();

void Dispatch();

void RegRead();

void Rename();

void Decode();

void Fetch();

uint8_t advance_cycle();