#include "../src/search/search.h"

int main()
{
	board b;
	b.init(start_position, true);

	negamax n;

	while (1)
	{
		uint32_t move = n.go(b, 6, false, false);
		b.make_move(move, false);
		system("cls");
		b.display();
		printf("EVALUATION: %d\n", b.evaluate());
		Sleep(300);
	}

	return 0;
}