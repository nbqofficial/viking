#pragma once

#include "../search/search.h"
#include "../perft/perft.h"

class uci
{
	private:

		perft pf;
		search sc;
		board b;

	public:

		uci();

		~uci();

		void go(char* line_in);

		void position(char* line_in);

		void uci_loop();
};