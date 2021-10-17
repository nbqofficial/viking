#pragma once

#include "../board/board.h"

class perft
{
	private:

		long long nodes = 0;

	public:

		void node_test(board& b, const uint8_t& depth);

		void test(board& b, const uint8_t& depth);
};