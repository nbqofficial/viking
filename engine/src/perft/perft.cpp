#include "perft.h"

void perft::node_test(board& b, const uint8_t& depth)
{
	if (!depth)
	{
		this->nodes++;
		return;
	}

	std::vector<uint32_t> moves;
	b.generate_moves(moves, false, all_moves);

	for (int i = 0; i < moves.size(); ++i)
	{
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.make_move(moves[i], false);
		node_test(b, depth - 1);
		b.restore_board(undo_board);
	}
	return;
}

void perft::test(board& b, const uint8_t& depth)
{
	b.display();
	printf("\tPERFORMANCE TESTING\n\n");
	printf("\tdepth         nodes         \n\n");
	for (int i = 1; i <= depth; ++i)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		node_test(b, i);
		auto stop_time = std::chrono::high_resolution_clock::now();
		auto exec_time = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
		printf("\t%d", i);
		printf("             %lld (%lld nps)\n", this->nodes, (long long)(this->nodes / (exec_time / 1000000.0)));
		this->nodes = 0;
	}
}
