#include "perft.h"

void perft::leaf_node_check(board& b, const uint8_t& depth)
{
	if (!depth)
	{
		this->leaf_nodes++;
		return;
	}

	std::vector<uint32_t> moves;
	b.generate_moves(moves, false, all_moves);

	for (int i = 0; i < moves.size(); ++i)
	{
		b.make_move(moves[i]);
		leaf_node_check(b, depth - 1);
		b.undo_move();
	}
	return;
}

void perft::check(board& b, const uint8_t& depth)
{
	b.display();
	printf("\t\t\t\tPERFORMANCE TESTING\n\n");
	printf("\t\t\tdepth         nodes         \n\n");
	for (int i = 1; i <= depth; ++i)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		leaf_node_check(b, i);
		auto stop_time = std::chrono::high_resolution_clock::now();
		auto exec_time = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
		printf("\t\t\t%d", i);
		printf("             %ld (%ld nps)\n", this->leaf_nodes, (long)(this->leaf_nodes / (exec_time / 1000000.0)));
		this->leaf_nodes = 0;
	}
}
