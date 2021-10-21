#pragma once

#include "../board/board.h"

class search
{
	private:

			long long nodes = 0;
			long long fh = 0;
			long long fhf = 0;

			int quiescence(board& b, int alpha, int beta);

			int negamax(board& b, int depth, double prob, int alpha, int beta, std::vector<uint32_t>& pv);

	public:

		uint32_t go(board& b, const int& depth, const bool& display_info, const bool& display_debug);
};