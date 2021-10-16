#pragma once

#include "../board/board.h"

class search
{
	private:

		long nodes = 0;

	public:

		int quiescence(board& b, int alpha, int beta);

		int srch(board& b, double p, int alpha, int beta, std::vector<uint32_t>& pv);

		uint32_t go(board& b, double p, const bool& display_pvs);
};