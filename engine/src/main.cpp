#include "../src/board/board.h"
#include "../src/perft/perft.h"

int main()
{
	board b;
	b.init("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

	perft p;
	p.check(b, 7);

	return 0;
}