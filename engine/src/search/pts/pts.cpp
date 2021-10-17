#include "pts.h"

int pts::quiescence(board& b, int alpha, int beta)
{
	this->nodes++;

	if (b.is_repetition() || b.get_fifty_move() >= 100) { return 0; }

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

int pts::pts_internal(board& b, double probability, int alpha, int beta, std::vector<uint32_t>& pv)
{
	if (probability < PROBABILITY_LIMIT) { return quiescence(b, alpha, beta); }

	this->nodes++;

	if (b.is_repetition() || b.get_fifty_move() >= 100) { return 0; }

	bool inchk = b.is_in_check();
	if (inchk) { probability *= 10; }

	int score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, all_moves);
	int moves_size = moves.size();

	if (!moves_size)
	{
		if (inchk) { return -MATE_SCORE - probability; }
		else { return 0; }
	}

	for (int i = 0; i < moves_size; ++i)
	{
		std::vector<uint32_t> childpv;
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.make_move(moves[i], false);
		score = -pts_internal(b, (probability / moves_size), -beta, -alpha, childpv);
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

uint32_t pts::go(board& b, const double& probability, const bool& display_pv, const bool& display_debug)
{
	if (display_pv) { printf("\tdepth     principal variation\n\n"); }

	uint32_t best_move = 0;
	int best_score = -INF_SCORE;
	int depth = 0;

	for (double curr_prob = (PROBABILITY_LIMIT * 10); curr_prob <= probability; curr_prob *= 10)
	{
		std::vector<uint32_t> newpv;
		best_score = pts_internal(b, curr_prob, -INF_SCORE, INF_SCORE, newpv);
		best_move = newpv[0];
		if (display_pv) { b.display_pv(newpv, depth); }

		if (display_debug)
		{
			printf("\tevaluation: %d\n", best_score);
			printf("\tmove ordering: %lld/%lld [%lld]\n", this->fhf, this->fh, this->nodes);
			printf("\tprobability: %f\n", curr_prob);
		}
		depth++;
	}

	this->nodes = 0;
	this->fh = 0;
	this->fhf = 0;

	return best_move;
}
