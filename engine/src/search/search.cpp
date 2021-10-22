#include "search.h"

int search::quiescence(board& b, int alpha, int beta)
{
	if ((this->nodes & 2047) == 0) { helper::check_up(); }

	this->nodes++;

	if (b.is_repetition() || b.get_fifty_move() >= 100) { return 0; }

	int score = b.evaluate();

	if (score >= beta) { return beta; }
	if (score > alpha) { alpha = score; }

	score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, only_forcing, true, 0);
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

int search::negamax(board& b, int depth, int alpha, int beta, std::vector<uint32_t>& pv, const bool& null_move)
{
	int score = -INF_SCORE;

	uint8_t hash_flag = tf_alpha;

	bool found_pv = false;

	if (depth <= 0) { return quiescence(b, alpha, beta); }

	if ((this->nodes & 2047) == 0) { helper::check_up(); }

	this->nodes++;

	if (b.is_repetition() || b.get_fifty_move() >= 100) { return 0; }

	int pv_node = beta - alpha > 1;

	score = this->transpo_table.read(b.get_hashkey(), depth, alpha, beta);
	if (score != VALUE_UNKNOWN && !pv_node) { return score; }

	bool inchk = b.is_in_check();
	if (inchk) { depth++; }

	score = -INF_SCORE;

	if (null_move && !inchk && depth >= NULL_MOVE_R)
	{
		std::vector<uint32_t> cpv;
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.switch_side();
		b.remove_enpassant();
		score = -negamax(b, depth - NULL_MOVE_R, -beta, -beta + 1, cpv, false);
		b.restore_board(undo_board);

		if (score >= beta)
		{
			this->null_cuttoff++;
			return beta;
		}
	}

	score = -INF_SCORE;

	std::vector<uint32_t> moves;
	b.generate_moves(moves, true, all_moves, true, depth - 1);
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
			score = -negamax(b, depth - 1, -alpha - 1, -alpha, childpv, true);
		
			if ((score > alpha) && (score < beta))
			{
				score = -negamax(b, depth - 1, -beta, -alpha, childpv, true);
			}
		}
		else
		{
			score = -negamax(b, depth - 1, -beta, -alpha, childpv, true);	
		}
		
		b.restore_board(undo_board);

		if (uci_info.stopped) { break; }

		if (score > alpha)
		{
			hash_flag = tf_exact;

			if (!b.get_move_capture_flag(moves[i])) { b.add_history_move(5, b.get_move_piece(moves[i]), b.get_move_to(moves[i])); }

			if (score >= beta)
			{
				this->transpo_table.write(b.get_hashkey(), depth, tf_beta, beta);

				if (!b.get_move_capture_flag(moves[i])) { b.add_killer_move(moves[i], depth - 1); }

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

	this->transpo_table.write(b.get_hashkey(), depth, hash_flag, alpha);

	return alpha;
}

search::search()
{
	if (this->transpo_table.allocate(32)) { this->transpo_table.reset(); }
}

search::~search()
{
	this->transpo_table.deallocate();
}

uint32_t search::go(board& b, const int& depth, const bool& display_info, const bool& display_debug)
{
	b.reset_killer_and_history_moves();

	uint32_t best_move = 0;
	int best_score = -INF_SCORE;

	for (int current_depth = 1; current_depth <= depth; ++current_depth)
	{
		b.pv_line.clear();

		best_score = negamax(b, current_depth, -INF_SCORE, INF_SCORE, b.pv_line, true);

		if (uci_info.stopped) { break; }

		best_move = b.pv_line[0];

		if (display_debug)
		{
			b.display_pv_debug(b.pv_line, current_depth);
			printf("\tevaluation: %d\n", best_score);
			printf("\tmove ordering: %f %lld/%lld [%lld]\n", (float)(((float)this->fhf / (float)this->fh) * 100.0), this->fhf, this->fh, this->nodes);
			printf("\tnull cuttoffs: %lld\n", this->null_cuttoff);
			printf("\tlate move reductions: %lld\n", this->lmr_count);
		}

		if (display_info) { b.display_info(b.pv_line, best_score, current_depth, this->nodes); }	
	}
	
	this->nodes = 0;
	this->fh = 0;
	this->fhf = 0;
	this->null_cuttoff = 0;

	helper::clear_searchinfo();
	return best_move;
}
