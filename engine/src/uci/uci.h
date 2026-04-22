#pragma once

#include <vector>
#include <memory>
#include <thread>

#include "../search/search.h"
#include "../perft/perft.h"

class uci
{
	private:

		perft pf;
		transpo tt;
		std::vector<std::unique_ptr<search>> workers;
		int threads = 1;
		size_t hash_mb = 256;
		board b;

		bool debug = false;
		bool display_after_move = false;

		void resize_workers();

		uint32_t run_search(int depth, bool display_info, bool display_debug);

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

		void parse_testpos(char* line_in);

		void parse_setoption(char* line_in);

		void uci_loop();
};
