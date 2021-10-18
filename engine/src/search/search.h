#pragma once

#include "../board/board.h"

class search
{
	private:

			long long nodes = 0;
			long long fh = 0;
			long long fhf = 0;
			long long null_cuttoff = 0;

			int quiescence(board& b, int alpha, int beta);

			int negamax(board& b, int depth, int alpha, int beta, std::vector<uint32_t>& pv, const bool& null_move);

	public:

		uint32_t go(board& b, const int& depth, const bool& display_pv, const bool& display_debug);
};