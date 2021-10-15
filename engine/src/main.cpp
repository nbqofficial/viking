#include "../src/board/board.h"

int main()
{
	board b;
	b.init(f6_enpassant_test, true);
	b.display();
	return 0;
}