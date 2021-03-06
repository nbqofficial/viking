#pragma once

#include "../search/search.h"
#include "../perft/perft.h"

class uci
{
	private:

		perft pf;
		search sc;
		board b;

		bool debug = false;
		bool display_after_move = false;

	public:

		uci();

		~uci();

		void go(char* line_in);

		void position(char* line_in);

		void parse_perft(char* line_in);

		void parse_displaymoves();

		void parse_displayboard();

		void parse_debug();

		void parse_displayaftermove();

		void uci_loop();
};