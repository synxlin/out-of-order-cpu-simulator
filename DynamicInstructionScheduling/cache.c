#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "cache.h"

/*
 *	Inital the cache
 */
void Init_Cache(uint32_t assoc)
{
	if (CACHE_SIZE == 0)
	{
		cache = NULL;
		return;
	}
	cache = (Cache*)malloc(sizeof(Cache));
	if (cache == NULL)
		_error_exit("malloc");
	cache->cacheAttributes.block_Width = log_2(blockSize);
	/* first, initial the attributes of cache */
	cache->cacheAttributes.size = CACHE_SIZE;
	cache->cacheAttributes.assoc = assoc;
	cache->cacheAttributes.set_Num = cache->cacheAttributes.size / (cache->cacheAttributes.assoc * blockSize);

	cache->cacheAttributes.index_Width = log_2(cache->cacheAttributes.set_Num);
	cache->cacheAttributes.tag_Width = 32 - cache->cacheAttributes.index_Width - cache->cacheAttributes.block_Width;

	/* then, allocate space for sets (including blocks and tag array) */
	cache->set = (Set *)malloc(sizeof(Set) * cache->cacheAttributes.set_Num);
	if (cache->set == NULL)
		_error_exit("malloc");
	uint32_t i;
	for (i = 0; i < cache->cacheAttributes.set_Num; i++)
	{
		cache->set[i].block = (Block *)malloc(sizeof(Block) * cache->cacheAttributes.assoc);
		if (cache->set[i].block == NULL)
			_error_exit("malloc");
		memset(cache->set[i].block, 0, sizeof(Block) * cache->cacheAttributes.assoc);
		cache->set[i].rank = (uint32_t *)malloc(sizeof(uint32_t) * cache->cacheAttributes.assoc);
		if (cache->set[i].rank == NULL)
			_error_exit("malloc");
		memset(cache->set[i].rank, 0, sizeof(uint32_t) * cache->cacheAttributes.assoc);
		cache->set[i].max_rank = 0;
	}
}

/*
 *	Interpreter the address into tag, index
 */
void Interpret_Address(uint32_t addr, uint32_t *tag, uint32_t *index)
{
	uint32_t tag_width = cache->cacheAttributes.tag_Width;
	*tag = addr >> (32 - tag_width);
	*index = (addr << tag_width) >> (tag_width + cache->cacheAttributes.block_Width);
}

/*
 *	Rebuild address from tag and index
 */
uint32_t Rebuild_Address(uint32_t tag, uint32_t index)
{
	uint32_t addr = 0;
	addr |= (tag << (cache->cacheAttributes.index_Width + cache->cacheAttributes.block_Width));
	addr |= (index << cache->cacheAttributes.block_Width);
	return addr;
}

/*
 *	Search the cache
 *	return the result: HIT or MISS
 *	if HIT, the hit way number will return by input parameter pointer "*way_num"
 */
uint32_t Cache_Search(uint32_t tag, uint32_t index)
{
	uint32_t i, k = cache->cacheAttributes.assoc;
	for (i = 0; i < cache->cacheAttributes.assoc; i++)
		if (cache->set[index].block[i].valid_Bit == VALID && cache->set[index].block[i].tag == tag)
		{
			k = i;
			break;
		}
	return k;
}

/*
 *	Maintain the rank array
 */
void Rank_Maintain(uint32_t index, uint32_t way_num)
{
	cache->set[index].max_rank++;
	cache->set[index].rank[way_num] = cache->set[index].max_rank;
#ifdef DBG
	{
		fprintf(debug_fp, "Rank: Cache Set %u -- ", index);
		uint32_t i;
		for (i = 0; i < cache->cacheAttributes.assoc; i++)
			fprintf(debug_fp, "%u ", cache->set[index].rank[i]);
		fprintf(debug_fp, "\n");
	}
#endif
}

/*
 *	Return the way number to be placed or replaced
 */
uint32_t Rank_Top(uint32_t index)
{
	uint32_t i, assoc = cache->cacheAttributes.assoc;
	/* we first use invalid block location */
	for (i = 0; i < assoc; i++)
		if (cache->set[index].block[i].valid_Bit == INVALID)
			return i;
	uint32_t *rank = cache->set[index].rank;
	uint32_t k = 0;
	for (i = 0; i < assoc; i++)
		if (rank[i] < rank[k])
			k = i;
	return k;
}

/*
 *	Allocate (Place or Replace) block "blk" on cache, set "index", way "way_num"
 */
void Cache_Replacement(uint32_t index, uint32_t way_num, uint32_t tag)
{
	cache->set[index].block[way_num].valid_Bit = VALID;
	cache->set[index].block[way_num].tag = tag;
	cache->set[index].block[way_num].cycle = MISS_PENALTY;
	cache->set[index].block[way_num].prefetch_Bit = NON_PREFETCH;
#ifdef DBG
	fprintf(debug_fp, "Replacement %x: Cache Set %u, Way %u\n", Rebuild_Address(tag, index), index, way_num);
#endif
}

void Cache_Prefetch(uint32_t addr)
{
	if (prefetch_flag == NON_PREFETCH)
		return;
	/* search this cache */
	uint32_t tag, index;
	Interpret_Address(addr, &tag, &index);
	uint32_t way_num = Cache_Search(tag, index);
	if (way_num < cache->cacheAttributes.assoc)
		return;
	way_num = Rank_Top(index);
	cache->set[index].block[way_num].valid_Bit = VALID;
	cache->set[index].block[way_num].tag = tag;
	cache->set[index].block[way_num].cycle = PREFETCH_PENALTY;
	cache->set[index].block[way_num].prefetch_Bit = PREFETCH;

	Rank_Maintain(index, way_num);
}
/*
 *	read operation
 *	block content is sent back by input parameter "*blk"
 */
Block Read(uint32_t addr)
{
	if (cache == NULL)
	{
		Block tmp;
		tmp.tag = 0;
		tmp.cycle = 0;
		tmp.prefetch_Bit = NON_PREFETCH;
		tmp.valid_Bit = VALID;
		return tmp;
	}
	/* search this cache */
	uint32_t tag, index;
	Interpret_Address(addr, &tag, &index);
	uint32_t way_num = Cache_Search(tag, index);

	/* if read hit */
	if (way_num < cache->cacheAttributes.assoc)
	{
#ifdef DBG
		fprintf(debug_fp, "Read %x : Cache Hit. Loc: Set %u, Way %u\n", addr, index, way_num);
#endif
		/* maintain the rank array */
		Rank_Maintain(index, way_num);
		Block tmp = cache->set[index].block[way_num];
		/* tmp.cycle = (tmp.prefetch_Bit == PREFETCH) ? tmp.cycle : 0; */
		if (cache->set[index].block[way_num].prefetch_Bit == PREFETCH)
		{
			cache->set[index].block[way_num].prefetch_Bit = NON_PREFETCH;
			Cache_Prefetch(addr + blockSize);
		}
		return tmp;
	}
	else
	{
#ifdef DBG
		fprintf(debug_fp, "Read %x : Cache Miss\n", addr);
#endif
		/* if read miss */
		way_num = Rank_Top(index);
		Cache_Replacement(index, way_num, tag);
		Rank_Maintain(index, way_num);
		Block tmp = cache->set[index].block[way_num];
		Cache_Prefetch(addr + blockSize);
		return tmp;
	}
}

void Cache_Update()
{
	if (cache == NULL)
		return;
	uint32_t i = 0;
	for (i = 0; i < cache->cacheAttributes.set_Num; i++)
	{
		uint32_t j = 0;
		for (j = 0; j < cache->cacheAttributes.assoc; j++)
		{
			if (cache->set[i].block[j].cycle > 0)
				cache->set[i].block[j].cycle--;
		}
	}
}

/*
* free space allocated for cache
*/
void Free_Cache()
{
	if (cache == NULL)
		return;
	uint32_t j;
	for (j = 0; j < cache->cacheAttributes.set_Num; j++)
	{
		free(cache->set[j].block);
		free(cache->set[j].rank);
	}
	free(cache->set);
}
