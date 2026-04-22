#pragma once

#include "../transpo/transpo.h"

class search
{
	private:

			long long nodes = 0;
			long long fh = 0;
			long long fhf = 0;
			long long null_cuttoff = 0;
			long long lmr_count = 0;
			long long transpo_cuttoff = 0;
			long long razoring_cuttoff = 0;

			transpo* tt = nullptr;
			int thread_id = 0;

			uint32_t pv_table[MAX_DEPTH + 1][MAX_DEPTH + 1];
			int pv_length[MAX_DEPTH + 1];
			int lmr_table[MAX_DEPTH + 1][64];

			int quiescence(board& b, int alpha, int beta);

			int negamax(board& b, int depth, int alpha, int beta, int ply, bool null_move);

	public:

		search();

		~search();

		void attach_tt(transpo* t) { this->tt = t; }

		void set_thread_id(int id) { this->thread_id = id; }

		bool is_main() const { return this->thread_id == 0; }

		uint32_t go(board& b, int depth, bool display_info, bool display_debug);
};
