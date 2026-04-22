#pragma once

#include "../board/board.h"
#include <atomic>

typedef struct _transposition_table
{
	std::atomic<uint64_t> key;   // real_hashkey XOR data (lockless hashing)
	std::atomic<uint64_t> data;  // packed: best_move(32) | depth(8) | flag(2) | score_biased(22)
}transposition_table, *ptransposition_table;

enum transpo_flags : uint8_t
{
	tf_exact, tf_alpha, tf_beta
};

class transpo
{
	private:

		ptransposition_table tt = nullptr;
		bool allocated = false;
		size_t size_in_mb = 0;
		size_t entries = 0;
		std::atomic<size_t> used{0};

	public:

		transpo();

		~transpo();

		bool allocate(size_t size_in_mb);

		bool deallocate();

		bool reset();

		int read(uint64_t hashkey, int depth, int alpha, int beta);

		uint32_t probe_move(uint64_t hashkey);

		bool write(uint64_t hashkey, int depth, uint8_t flag, int score, uint32_t best_move);

		size_t get_used() const { return used.load(std::memory_order_relaxed); }

		size_t get_entries() const { return entries; }
};
