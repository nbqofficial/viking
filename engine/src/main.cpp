#include "perft/perft.h"

int main()
{
	board b;
	b.init(start_position, true);
	b.display();
	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, all_moves);
	b.display_moves(moves);

	return 0;
}