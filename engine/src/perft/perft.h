#pragma once

#include "../board/board.h"

class perft
{
	private:

		long leaf_nodes = 0;

	public:

		void leaf_node_check(board& b, const uint8_t& depth);

		void check(board& b, const uint8_t& depth);
};