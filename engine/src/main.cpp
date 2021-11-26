#include "../src/uci/uci.h"

int main()
{
	//uci u;
	//u.uci_loop();

	uint64_t bb1 = 0ULL;
	uint64_t bb2 = 0ULL;

	bitwise::set(bb1, 30);
	bitwise::set(bb2, 22);

	bitwise::display(bb1 & bb2);

	return 0;
}