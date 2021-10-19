#include "search.h"

int search::quiescence(board& b, int alpha, int beta)
{
	if ((this->nodes & 2047) == 0) { helper::check_up(); }

	this->nodes++;

	if (b.is_repetition()) { return 0; }

	int score = b.evaluate();

	if (score >= beta) { return beta; }
	if (score > alpha) { alpha = score; }

	score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, only_forcing, true);
	int moves_size = moves.size();

	for (int i = 0; i < moves_size; ++i)
	{
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.make_move(moves[i], false);
		score = -quiescence(b, -beta, -alpha);
		b.restore_board(undo_board);

		if (uci_info.stopped) { break; }

		if (score > alpha)
		{
			if (score >= beta) 
			{ 
				if (i == 0) { this->fhf++; }
				this->fh++;
				return beta; 
			}
			alpha = score;
		}
	}
	return alpha;
}

int search::negamax(board& b, const int& depth, double prob, int alpha, int beta, std::vector<uint32_t>& pv)
{
	bool found_pv = false;

	if (prob < LOW_PROBABILITY_LIMIT) { return quiescence(b, alpha, beta); }

	if ((this->nodes & 2047) == 0) { helper::check_up(); }

	this->nodes++;

	if (b.is_repetition()) { return 0; }

	bool inchk = b.is_in_check();
	if (inchk) { prob *= 10; }

	int score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, all_moves, true);
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

		if (found_pv)
		{
			score = -negamax(b, depth - 1, (prob / moves_size), -alpha - 1, -alpha, childpv);
		
			if ((score > alpha) && (score < beta))
			{
				score = -negamax(b, depth - 1, (prob / moves_size), -beta, -alpha, childpv);
			}
		}
		else
		{
			score = -negamax(b, depth - 1, (prob / moves_size), -beta, -alpha, childpv);
		}
		
		b.restore_board(undo_board);

		if (uci_info.stopped) { break; }

		if (score > alpha)
		{
			if (score >= beta)
			{
				if (i == 0) { this->fhf++; }
				this->fh++;
				return beta;
			}
			alpha = score;
			found_pv = true;

			pv.clear();
			pv.push_back(moves[i]);
			std::copy(childpv.begin(), childpv.end(), std::back_inserter(pv));
		}
	}
	return alpha;
}

uint32_t search::go(board& b, const int& depth, const bool& display_info, const bool& display_debug)
{
	uint32_t best_move = 0;
	int best_score = -INF_SCORE;

	for (int current_depth = 1; current_depth <= depth; ++current_depth)
	{
		std::vector<uint32_t> newpv;
		double prob = (LOW_PROBABILITY_LIMIT * pow(10, current_depth));
		best_score = negamax(b, current_depth, prob, -INF_SCORE, INF_SCORE, newpv);

		if (uci_info.stopped) { break; }

		best_move = newpv[0];

		if (display_debug)
		{
			b.display_pv_debug(newpv, current_depth);
			printf("\tprobability: %f\n", prob);
			printf("\tevaluation: %d\n", best_score);
			printf("\tmove ordering: %lld/%lld [%lld]\n", this->fhf, this->fh, this->nodes);
			printf("\tnull cuttoffs: %lld\n", this->null_cuttoff);
		}

		if (display_info) { b.display_info(newpv, best_score, current_depth, this->nodes); }

		this->nodes = 0;
		this->fh = 0;
		this->fhf = 0;
		this->null_cuttoff = 0;
	}
	helper::clear_searchinfo();
	return best_move;
}
