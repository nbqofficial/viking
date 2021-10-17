#pragma once

#include "../../board/board.h"

class pts
{
	private:

		long long nodes = 0;
		long long fh = 0;
		long long fhf = 0;

		int quiescence(board& b, int alpha, int beta);

		int pts_internal(board& b, double probability, int alpha, int beta, std::vector<uint32_t>& pv);

	public:

		uint32_t go(board& b, const double& probability, const bool& display_pv, const bool& display_debug);
};