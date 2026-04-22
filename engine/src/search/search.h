#pragma once

#include "../transpo/transpo.h"

class search
{
	private:

			long long nodes = 0;
			long long fh = 0;
			long long fhf = 0;
			long long null_cuttoff = 0;
			long long lmr_count = 0;
			long long transpo_cuttoff = 0;
			long long razoring_cuttoff = 0;
			transpo transpo_table;

			uint32_t pv_table[MAX_DEPTH + 1][MAX_DEPTH + 1];
			int pv_length[MAX_DEPTH + 1];
			int lmr_table[MAX_DEPTH + 1][64];

			int quiescence(board& b, int alpha, int beta);

			int negamax(board& b, int depth, int alpha, int beta, int ply, bool null_move);

	public:

		search();

		~search();

		uint32_t go(board& b, int depth, bool display_info, bool display_debug);
};