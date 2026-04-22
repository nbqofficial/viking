#include "transpo.h"

// Packing: bits 0..31 best_move, 32..39 depth, 40..41 flag, 42..63 score_biased (22 bits).
// Score range used by search is within [-INF_SCORE, +INF_SCORE]; biased to unsigned.
static inline uint64_t pack_data(uint32_t best_move, int depth, uint8_t flag, int score) noexcept
{
	uint64_t d = (uint64_t)best_move;
	d |= ((uint64_t)(uint8_t)(depth & 0xFF)) << 32;
	d |= ((uint64_t)(flag & 0x3)) << 40;
	uint32_t sb = (uint32_t)(score + INF_SCORE) & 0x3FFFFF;
	d |= ((uint64_t)sb) << 42;
	return d;
}

static inline void unpack_data(uint64_t d, uint32_t& best_move, int& depth, uint8_t& flag, int& score) noexcept
{
	best_move = (uint32_t)(d & 0xFFFFFFFFULL);
	depth = (int)(int8_t)((d >> 32) & 0xFF);
	flag = (uint8_t)((d >> 40) & 0x3);
	score = (int)((d >> 42) & 0x3FFFFF) - INF_SCORE;
}

transpo::transpo()
{
}

transpo::~transpo()
{
}

bool transpo::allocate(size_t size_in_mb)
{
	if (this->allocated) { return false; }
	this->size_in_mb = size_in_mb;
	this->entries = ((size_in_mb * 0x100000) / sizeof(transposition_table));
	this->tt = new transposition_table[this->entries]();
	this->used.store(0, std::memory_order_relaxed);
	this->allocated = true;
	return true;
}

bool transpo::deallocate()
{
	if (!this->allocated) { return false; }
	delete[] this->tt;
	this->tt = nullptr;
	this->size_in_mb = 0;
	this->entries = 0;
	this->used.store(0, std::memory_order_relaxed);
	this->allocated = false;
	return true;
}

bool transpo::reset()
{
	if (!this->allocated || !this->size_in_mb) { return false; }

	for (size_t i = 0; i < this->entries; ++i)
	{
		this->tt[i].key.store(0, std::memory_order_relaxed);
		this->tt[i].data.store(0, std::memory_order_relaxed);
	}
	this->used.store(0, std::memory_order_relaxed);
	return true;
}

uint32_t transpo::probe_move(uint64_t hashkey)
{
	ptransposition_table entry = &this->tt[hashkey % this->entries];
	uint64_t d = entry->data.load(std::memory_order_relaxed);
	uint64_t k = entry->key.load(std::memory_order_relaxed);
	if ((k ^ d) != hashkey) { return 0; }
	return (uint32_t)(d & 0xFFFFFFFFULL);
}

int transpo::read(uint64_t hashkey, int depth, int alpha, int beta)
{
	ptransposition_table entry = &this->tt[hashkey % this->entries];

	uint64_t d = entry->data.load(std::memory_order_relaxed);
	uint64_t k = entry->key.load(std::memory_order_relaxed);

	if ((k ^ d) != hashkey) { return VALUE_UNKNOWN; }

	uint32_t bm; int ed; uint8_t fl; int sc;
	unpack_data(d, bm, ed, fl, sc);

	if (ed == depth)
	{
		if (fl == tf_exact) { return sc; }
		if ((fl == tf_alpha) && (sc <= alpha)) { return alpha; }
		if ((fl == tf_beta) && (sc >= beta)) { return beta; }
	}

	return VALUE_UNKNOWN;
}

bool transpo::write(uint64_t hashkey, int depth, uint8_t flag, int score, uint32_t best_move)
{
	ptransposition_table entry = &this->tt[hashkey % this->entries];

	uint64_t old_d = entry->data.load(std::memory_order_relaxed);
	uint64_t old_k = entry->key.load(std::memory_order_relaxed);

	if (old_k == 0 && old_d == 0) { this->used.fetch_add(1, std::memory_order_relaxed); }

	uint64_t d = pack_data(best_move, depth, flag, score);
	entry->key.store(hashkey ^ d, std::memory_order_relaxed);
	entry->data.store(d, std::memory_order_relaxed);

	return true;
}
