#include "../src/search/search.h"

int main()
{	
	board b;
	b.init("r1b1kb1r/2p2ppp/p1p2q2/3p2N1/3Qn3/8/PPP2PPP/RNB1R1K1 w kq - 2 10", true);
	b.display();

	pts p;

	uint32_t move = p.go(b, 100.0, true, true);

	return 0;
}