#include "search.h"
#include <cmath>

int search::quiescence(board& b, int alpha, int beta)
{
	if ((this->nodes & 2047) == 0) { helper::check_up(); }

	this->nodes++;

	int score = b.evaluate();

	if (score >= beta) { return beta; }
	if (score > alpha) { alpha = score; }

	score = -INF_SCORE;

	move_list moves{};
	b.generate_moves(moves, true, only_forcing, true, 0);
	int moves_size = moves.m_size;

	for (int i = 0; i < moves_size; ++i)
	{
		board_delta delta;
		b.make_move(moves.m_moves[i], delta);

		if (moves_size > 1 && b.is_repetition())
		{
			b.unmake_move(moves.m_moves[i], delta);
			continue;
		}

		score = -quiescence(b, -beta, -alpha);
		b.unmake_move(moves.m_moves[i], delta);

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

int search::negamax(board& b, int depth, int alpha, int beta, int ply, bool null_move)
{
	int score = -INF_SCORE;

	uint8_t hash_flag = tf_alpha;

	int moves_searched = 0;
	bool found_pv = false;

	this->pv_length[ply] = ply;

	if (depth <= 0) { return quiescence(b, alpha, beta); }

	if ((this->nodes & 2047) == 0) { helper::check_up(); }

	this->nodes++;

	int pv_node = beta - alpha > 1;

	bool inchk = b.is_in_check();
	if (inchk) { depth++; }
	
#ifdef _USE_NNUE
	if (!pv_node && !inchk && depth == 1)
	{
		int stand_pat = b.evaluate();
		const int RAZOR_MARGIN = 150;
		if (stand_pat + RAZOR_MARGIN <= alpha)
		{ 
			this->razoring_cuttoff++;
			return quiescence(b, alpha, beta); 
		}
	}
#endif

	score = this->transpo_table.read(b.get_hashkey(), depth, alpha, beta);
	if (score != VALUE_UNKNOWN && !pv_node)
	{
		this->transpo_cuttoff++;
		return score;
	}

	uint32_t tt_move = this->transpo_table.probe_move(b.get_hashkey());

	score = -INF_SCORE;

	if (null_move && !inchk && depth >= NULL_MOVE_R && b.big_piece_exist())
	{
		board_undo undo_board;
		b.preserve_board(undo_board);
		b.switch_side();
		b.remove_enpassant();
		score = -negamax(b, depth - NULL_MOVE_R, -beta, -beta + 1, ply + 1, false);
		b.restore_board(undo_board);

		if (score >= beta)
		{
			this->null_cuttoff++;
			return beta;
		}
	}

	score = -INF_SCORE;

	move_list moves{};
	b.generate_moves(moves, true, all_moves, true, depth - 1);
	int moves_size = moves.m_size;

	if (!moves_size)
	{
		if (inchk) { return -MATE_SCORE + (MAX_DEPTH - depth); }
		else { return 0; }
	}

	if (tt_move)
	{
		for (int i = 0; i < moves_size; ++i)
		{
			if (moves.m_moves[i] == tt_move)
			{
				if (i != 0) { std::swap(moves.m_moves[0], moves.m_moves[i]); }
				break;
			}
		}
	}

	uint32_t best_move_local = 0;

	for (int i = 0; i < moves_size; ++i)
	{
		if (depth >= NULL_MOVE_R && i != 0 && !n_move::get_move_score(moves.m_moves[i])) { continue; }

		board_delta delta;
		b.make_move(moves.m_moves[i], delta);

		if (moves_size > 1 && b.is_repetition())
		{
			b.unmake_move(moves.m_moves[i], delta);
			continue;
		}

		if (found_pv)
		{
			score = -negamax(b, depth - 1, -alpha - 1, -alpha, ply + 1, true);

			if ((score > alpha) && (score < beta))
			{
				score = -negamax(b, depth - 1, -beta, -alpha, ply + 1, true);
			}
		}
		else
		{
			if (!moves_searched)
			{
				score = -negamax(b, depth - 1, -beta, -alpha, ply + 1, true);
			}
			else
			{
				if (moves_searched >= LMR_MOVE_LIMIT && depth >= LMR_DEPTH_LIMIT && !inchk && !n_move::get_move_capture_flag(moves.m_moves[i]) && !n_move::get_move_promoted_piece(moves.m_moves[i]))
				{
					int lmr_d = depth < MAX_DEPTH ? depth : MAX_DEPTH;
					int lmr_m = moves_searched < 63 ? moves_searched : 63;
					int R = this->lmr_table[lmr_d][lmr_m];
					if (pv_node && R > 0) { R--; }
					int reduced_depth = depth - 1 - R;
					if (reduced_depth < 1) { reduced_depth = 1; }

					score = -negamax(b, reduced_depth, -alpha - 1, -alpha, ply + 1, true);
					lmr_count++;
				}
				else
				{
					score = alpha + 1;
				}

				if (score > alpha)
				{
					score = -negamax(b, depth - 1, -alpha - 1, -alpha, ply + 1, true);

					if ((score > alpha) && (score < beta))
					{
						score = -negamax(b, depth - 1, -beta, -alpha, ply + 1, true);
					}
				}
			}
		}

		b.unmake_move(moves.m_moves[i], delta);

		if (uci_info.stopped) { break; }

		moves_searched++;

		if (score > alpha)
		{
			hash_flag = tf_exact;
			best_move_local = moves.m_moves[i];

			if (!n_move::get_move_capture_flag(moves.m_moves[i])) { b.add_history_move(depth, n_move::get_move_piece(moves.m_moves[i]), n_move::get_move_to(moves.m_moves[i])); }

			if (score >= beta)
			{
				this->transpo_table.write(b.get_hashkey(), depth, tf_beta, beta, moves.m_moves[i]);

				if (!n_move::get_move_capture_flag(moves.m_moves[i])) { b.add_killer_move(moves.m_moves[i], depth - 1); }

				if (i == 0) { this->fhf++; }
				this->fh++;
				return beta;
			}
			alpha = score;
			found_pv = true;

			this->pv_table[ply][ply] = moves.m_moves[i];
			for (int next = ply + 1; next < this->pv_length[ply + 1]; ++next)
			{
				this->pv_table[ply][next] = this->pv_table[ply + 1][next];
			}
			this->pv_length[ply] = this->pv_length[ply + 1];
		}
	}

	this->transpo_table.write(b.get_hashkey(), depth, hash_flag, alpha, best_move_local);

	return alpha;
}

search::search()
{
	if (this->transpo_table.allocate(256)) { this->transpo_table.reset(); }

	for (int d = 0; d <= MAX_DEPTH; ++d)
	{
		for (int m = 0; m < 64; ++m)
		{
			if (d < 1 || m < 1) { this->lmr_table[d][m] = 0; }
			else { this->lmr_table[d][m] = (int)(0.75 + std::log((double)d) * std::log((double)m) / 2.25); }
		}
	}
}

search::~search()
{
	this->transpo_table.deallocate();
}

uint32_t search::go(board& b, int depth, bool display_info, bool display_debug)
{
	b.reset_killer_and_history_moves();

	uint32_t best_move = 0;
	int best_score = -INF_SCORE;

	for (int current_depth = 1; current_depth <= depth; ++current_depth)
	{
		int alpha = -INF_SCORE;
		int beta = INF_SCORE;
		int window = 25;
		int near_mate = MATE_SCORE - MAX_DEPTH;

		if (current_depth >= 4 && std::abs(best_score) < near_mate)
		{
			alpha = best_score - window;
			beta = best_score + window;
		}

		int score = 0;
		while (true)
		{
			memset(this->pv_length, 0, sizeof(this->pv_length));

			score = negamax(b, current_depth, alpha, beta, 0, true);

			if (uci_info.stopped) { break; }

			if (score <= alpha)
			{
				beta = (alpha + beta) / 2;
				window *= 2;
				alpha = (score - window > -INF_SCORE) ? score - window : -INF_SCORE;
			}
			else if (score >= beta)
			{
				window *= 2;
				beta = (score + window < INF_SCORE) ? score + window : INF_SCORE;
			}
			else
			{
				break;
			}
		}

		best_score = score;

		if (uci_info.stopped) { break; }

		b.pv_line.clear();
		b.pv_line.reserve(this->pv_length[0]);
		for (int i = 0; i < this->pv_length[0]; ++i)
		{
			b.pv_line.push_back(this->pv_table[0][i]);
		}

		if (!b.pv_line.empty()) { best_move = b.pv_line[0]; }

		if (display_debug)
		{
			b.display_pv_debug(b.pv_line, current_depth);
			printf("\tevaluation: %d\n", best_score);
			printf("\tmove ordering: %f %lld/%lld [%lld]\n", (float)(((float)this->fhf / (float)this->fh) * 100.0), this->fhf, this->fh, this->nodes);
			printf("\tnull cuttoffs: %lld\n", this->null_cuttoff);
			printf("\tlate move reductions: %lld\n", this->lmr_count);
			printf("\ttransposition cuttoffs: %lld\n", this->transpo_cuttoff);
			printf("\trazoring cuttoffs: %lld\n", this->razoring_cuttoff);
		}

		if (display_info)
		{
			b.display_info(b.pv_line, best_score, current_depth, this->nodes);
			size_t tt_used = this->transpo_table.get_used();
			size_t tt_total = this->transpo_table.get_entries();
			int permille = tt_total ? (int)((tt_used * 1000) / tt_total) : 0;
			printf("info depth %d hashfull %d tthash %zu tttotal %zu\n",
			       current_depth, permille, tt_used, tt_total);
		}
	}

	this->nodes = 0;
	this->fh = 0;
	this->fhf = 0;
	this->null_cuttoff = 0;
	this->lmr_count = 0;
	this->transpo_cuttoff = 0;
	this->razoring_cuttoff = 0;

	helper::clear_searchinfo();
	return best_move;
}
