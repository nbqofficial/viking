#include "search.h"

int search::quiescence(board& b, int alpha, int beta)
{
	if (!(this->nodes & 2047)) { helper::checkup(); }
	this->nodes++;

	int score = b.evaluate();

	if (score >= beta) { return beta; }
	if (score > alpha) { alpha = score; }

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, only_captures);
	int moves_size = moves.size();

	score = -INF_SCORE;

	for (int i = 0; i < moves_size; ++i)
	{
		b.make_move(moves[i]);
		score = -quiescence(b, -beta, -alpha);
		b.undo_move();

		if (searchinfo.stopped) { break; }

		if (score > alpha)
		{
			if (score >= beta) { return beta; }
			alpha = score;
		}
	}
	return alpha;
}

int search::srch(board& b, double p, int alpha, int beta, std::vector<uint32_t>& pv)
{
	if (p < PROBABILITY_LIMIT) { return quiescence(b, alpha, beta);  }

	if (!(this->nodes & 2047)) { helper::checkup(); }
	
	bool inchk = b.is_in_check();

	int score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, all_moves);
	int moves_size = moves.size();

	if (moves.size() == 0)
	{
		if (inchk) { return (-MATE_SCORE - p); }
		else { return 0; }
	}

	for (int i = 0; i < moves_size; ++i)
	{
		std::vector<uint32_t> childpv;
		b.make_move(moves[i]);
		score = -srch(b, (p / moves_size), -beta, -alpha, childpv);
		b.undo_move();

		if (searchinfo.stopped) { break; }

		if (score > alpha)
		{
			if (score >= beta)
			{
				if (i == 0) { searchinfo.fhf++; }
				searchinfo.fh++;
				return beta;
			}
			alpha = score;
			pv.clear();
			pv.push_back(moves[i]);
			std::copy(childpv.begin(), childpv.end(), std::back_inserter(pv));
		}
	}
	return alpha;
}

uint32_t search::go(board& b, double p, const bool& display_pvs)
{
	uint32_t best_move = 0UL;
	int best_score = -INF_SCORE;
	int depth = 0;

	if (display_pvs) { printf("\tdepth     principal variation\n\n"); }

	for (double i = PROBABILITY_LIMIT; i <= p; i *= 10)
	{
		printf("a\n");
		std::vector<uint32_t> newpv;
		best_score = srch(b, i, -INF_SCORE, INF_SCORE, newpv);

		if (searchinfo.stopped) { break; }

		best_move = newpv[0];
		if (display_pvs) { b.display_pv(newpv, depth); }
		depth++;
	}
	helper::clear_search_info();
	this->nodes = 0;
	return best_move;
}
