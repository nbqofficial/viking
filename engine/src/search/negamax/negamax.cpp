#include "negamax.h"

int negamax::quiescence(board& b, int alpha, int beta)
{
	this->nodes++;

	int score = b.evaluate();

	if (score >= beta) { return beta; }
	if (score > alpha) { alpha = score; }

	score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, only_captures);
	int moves_size = moves.size();

	for (int i = 0; i < moves_size; ++i)
	{
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.make_move(moves[i], false);
		score = -quiescence(b, -beta, -alpha);
		b.restore_board(undo_board);

		if (score > alpha)
		{
			if (score >= beta) { return beta; }
			alpha = score;
		}
	}
	return alpha;
}

int negamax::nmax(board& b, int depth, int alpha, int beta, std::vector<uint32_t>& pv)
{
	if (depth <= 0) { return quiescence(b, alpha, beta); }

	bool inchk = b.is_in_check();

	int score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, all_moves);
	int moves_size = moves.size();

	if (!moves_size)
	{
		if (inchk) { return -MATE_SCORE + (MAX_DEPTH - depth); }
		else { return 0; }
	}

	for (int i = 0; i < moves_size; ++i)
	{
		std::vector<uint32_t> childpv;
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.make_move(moves[i], false);
		score = -nmax(b, depth - 1, -beta, -alpha, childpv);
		b.restore_board(undo_board);

		if (score > alpha)
		{
			if (score >= beta) 
			{ 
				if (i == 0) { this->fhf++; }
				this->fh++;
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

uint32_t negamax::go(board& b, const int& depth, const bool& display_pv, const bool& display_debug)
{
	if (display_pv) { printf("\tdepth     principal variation\n\n"); }

	uint32_t best_move = 0;
	int best_score = -INF_SCORE;

	for (int current_depth = 1; current_depth <= depth; ++current_depth)
	{
		std::vector<uint32_t> newpv;
		best_score = nmax(b, current_depth, -INF_SCORE, INF_SCORE, newpv);
		best_move = newpv[0];
		if (display_pv) { b.display_pv(newpv, current_depth); }

		if (display_debug) { printf("\tmove ordering: %lld/%lld [%lld]\n", this->fhf, this->fh, this->nodes); }
	}

	this->nodes = 0;
	this->fh = 0;
	this->fhf = 0;

	return best_move;
}
