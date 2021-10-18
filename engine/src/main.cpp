#include "../src/search/search.h"

int main()
{	
	search s;
	board b;
	b.init(start_position, false);
	b.display();

	std::vector<uint32_t> moves;
	b.generate_moves(moves, false, all_moves);

	while (moves.size())
	{
		moves.clear();
		b.generate_moves(moves, false, all_moves);
		if (moves.size() == 0 || b.get_fifty_move() >= 100) { break; }
		uint32_t move = s.go(b, 3, false, false);
		b.make_move(move, true);
		system("cls");
		b.display();
		Sleep(50);
	}
	if (b.is_in_check())
	{
		printf("%s WON\n", b.get_side() ? "WHITE" : "BLACK");
	}
	else
	{
		printf("DRAW\n");
	}
	return 0;
}