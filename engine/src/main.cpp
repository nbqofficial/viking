#include "../src/search/search.h"

int main()
{	
	board b;
	b.init("rnbq1r1k/pp1npPbp/3p4/4P3/5P2/2p2N2/PPP3P1/R1BQKB1R w KQ - 0 1", true);
	b.display();

	pts p;

	uint32_t move = p.go(b, 100.0, true, true);

	return 0;
}