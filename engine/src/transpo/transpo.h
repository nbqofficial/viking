#pragma once

#include "../board/board.h"

typedef struct _transposition_table
{
	uint64_t hashkey;
	int depth;
	uint8_t flag;
	int score;
}transposition_table, *ptransposition_table;

enum transpo_flags : uint8_t
{
	tf_exact, tf_alpha, tf_beta
};

class transpo
{
	private:

		ptransposition_table tt;
		bool allocated = false;
		size_t size_in_mb = 0;
		size_t entries = 0;

	public:

		transpo();

		~transpo();

		bool allocate(size_t size_in_mb);

		bool deallocate();

		bool reset();

		int read(uint64_t hashkey, int depth, int alpha, int beta);

		bool write(uint64_t hashkey, int depth, uint8_t flag, int score);
};