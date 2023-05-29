#include "../src/uci/uci.h"

int main()
{
	srand(time(0));

	uci u;
	u.uci_loop();

	return 0;
}