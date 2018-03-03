#pragma once
#ifndef CACHE_H_
#define CACHE_H_

#define HIT 0
#define MISS 1

#define PREFETCH 1
#define NON_PREFETCH 0

#define MISS_PENALTY 10
#define PREFETCH_PENALTY 10

typedef struct Block
{
	/* uint8_t* BLOCK_content; */
	uint32_t tag;
	uint8_t valid_Bit;
	int cycle;
	uint8_t prefetch_Bit;
}Block;

typedef struct Set
{
	Block* block;
	uint32_t* rank;
	uint32_t max_rank;
}Set;

typedef struct Cache_Attributes
{
	uint32_t size;
	uint32_t assoc;

	uint32_t set_Num;
	uint32_t tag_Width;
	uint32_t index_Width;
	uint32_t block_Width;
}Cache_Attributes;

typedef struct Cache
{
	Set* set;
	Cache_Attributes cacheAttributes;
}Cache;

extern Cache* cache;

extern uint32_t CACHE_SIZE;
extern uint32_t blockSize;

extern uint8_t prefetch_flag;

#ifdef DBG
extern FILE *debug_fp;
#endif

void Init_Cache(uint32_t assoc);

void Interpret_Address(uint32_t addr, uint32_t *tag, uint32_t *index);

uint32_t Rebuild_Address(uint32_t tag, uint32_t index);

uint32_t Cache_Search(uint32_t tag, uint32_t index);

void Rank_Maintain(uint32_t index, uint32_t way_num);

uint32_t Rank_Top(uint32_t index);

void Cache_Replacement(uint32_t index, uint32_t way_num, uint32_t tag);

void Cache_Prefetch(uint32_t addr);

Block Read(uint32_t addr);

void Cache_Update();

void Free_Cache();

#endif
