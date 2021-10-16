#include "../src/search/search.h"

int main()
{
	board b;
	b.init(wac1);
	b.display();

	search s;
	s.go(b, 10, true);

	return 0;
}