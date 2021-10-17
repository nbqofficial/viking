#pragma once

#include "../../board/board.h"

class negamax
{
	private:

		long long nodes = 0;
		long long fh = 0;
		long long fhf = 0;

		int quiescence(board& b, int alpha, int beta);

		int negamax_internal(board& b, int depth, int alpha, int beta, std::vector<uint32_t>& pv);

	public:

		uint32_t go(board& b, const int& depth, const bool& display_pv, const bool& display_debug);
};