#include "../src/uci/uci.h"

/*
*	  EVALUATION TODO
* 
*	- material
*	- piece square tables (opening and endgame)
*	- piece mobility
*	- bishop pair bonus
*	- bishop on opponent weak color complex bonus
*	- passed pawns bonus
*	- isolated pawns penalty
*	- doubled pawns penalty
*	- rook on open or semiopen file bonus
*	- open or semiopen file in front of the king penalty
*	- is king castled bonus
*/

int main()
{	
	uci u;
	u.uci_loop();

	return 0;
}