#include "../src/search/search.h"

int main()
{	
	board b;
	b.init(wac1, true);
	b.display();

	negamax p;

	uint32_t move = p.go(b, 8, true, false);

	return 0;
}