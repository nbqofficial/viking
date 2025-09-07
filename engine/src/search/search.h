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
			transpo transpo_table;

			int quiescence(board& b, int alpha, int beta);

			int negamax(board& b, int depth, int alpha, int beta, std::vector<uint32_t>& pv, bool null_move);

	public:

		search();

		~search();

		uint32_t go(board& b, int depth, bool display_info, bool display_debug);
};