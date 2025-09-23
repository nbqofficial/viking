#pragma once

#include "../move/move.h"
#include "../nnue/nnue.h"

class board
{
	private:

		uint64_t state[12];
		uint64_t occupied[3];
		uint8_t side;
		uint8_t castling;
		uint8_t enpassant;
		uint8_t fifty_move;
		uint64_t hashkey;
		std::vector<board_undo> history;
		uint32_t killer_moves[2][64];
		uint8_t history_moves[12][64];

	public:
		
		std::vector<uint32_t> pv_line;
		
		board() noexcept;

		~board() noexcept;

		bool reset() noexcept
		{
			memset(this->state, 0ULL, sizeof(this->state));
			memset(this->occupied, 0ULL, sizeof(this->occupied));
			this->side = white;
			this->castling = 0;
			this->enpassant = no_sq;
			this->fifty_move = 0;
			this->hashkey = 0;
			this->history.clear();
			memset(this->history_moves, 0, sizeof(this->history_moves));
			this->pv_line.clear();
			return true;
		}

		bool init(const std::string& fen, bool display_fen) noexcept;

		void display() noexcept;

		void display_move(uint32_t move) const noexcept;

		void display_moves(const move_list& moves) const noexcept;

		void display_pv_debug(const std::vector<uint32_t>& pv, int depth) const noexcept;

		void display_info(const std::vector<uint32_t>& pv, int score, int depth, long long nodes) const noexcept;

#ifdef _USE_MAGIC_BITBOARDS
		inline uint64_t rook_attacks_magic(uint8_t square) const noexcept
		{
			uint64_t occupancy = this->occupied[both];
			occupancy &= rook_masks[square];
			occupancy *= rook_magic_numbers[square];
			occupancy >>= 64 - rook_relevant_bits[square];

			return rook_attacks[square][occupancy];
		}

		inline uint64_t bishop_attacks_magic(uint8_t square) const noexcept
		{
			uint64_t occupancy = this->occupied[both];
			occupancy &= bishop_masks[square];
			occupancy *= bishop_magic_numbers[square];
			occupancy >>= 64 - bishop_relevant_bits[square];

			return bishop_attacks[square][occupancy];
		}

		inline uint64_t queen_attacks_magic(uint8_t square) const noexcept
		{
			uint64_t queen_attacks = 0ULL;
			uint64_t bishop_occupancy = this->occupied[both];
			uint64_t rook_occupancy = this->occupied[both];

			bishop_occupancy &= bishop_masks[square];
			bishop_occupancy *= bishop_magic_numbers[square];
			bishop_occupancy >>= 64 - bishop_relevant_bits[square];

			queen_attacks = bishop_attacks[square][bishop_occupancy];

			rook_occupancy &= rook_masks[square];
			rook_occupancy *= rook_magic_numbers[square];
			rook_occupancy >>= 64 - rook_relevant_bits[square];

			queen_attacks |= rook_attacks[square][rook_occupancy];

			return queen_attacks;
		}
#else
		inline uint64_t rook_attacks_hq(uint8_t square) const noexcept
		{
			uint64_t bin = 1ULL << square;
			uint64_t hor = (this->occupied[both] - 2 * bin) ^ bitwise::reverse(bitwise::reverse(this->occupied[both]) - 2 * bitwise::reverse(bin));
			uint64_t ver = ((this->occupied[both] & file_masks[square % 8]) - (2 * bin)) ^ bitwise::reverse(bitwise::reverse(this->occupied[both] & file_masks[square % 8]) - (2 * bitwise::reverse(bin)));
			return (hor & rank_masks[square / 8]) | (ver & file_masks[square % 8]);
		}

		inline uint64_t bishop_attacks_hq(uint8_t square) const noexcept
		{
			uint64_t bin = 1ULL << square;
			uint64_t diag = ((this->occupied[both] & diag_masks[(square / 8) + (square % 8)]) - (2 * bin)) ^ bitwise::reverse(bitwise::reverse(this->occupied[both] & diag_masks[(square / 8) + (square % 8)]) - (2 * bitwise::reverse(bin)));
			uint64_t antidiag = ((this->occupied[both] & antidiag_masks[(square / 8) + 7 - (square % 8)]) - (2 * bin)) ^ bitwise::reverse(bitwise::reverse(this->occupied[both] & antidiag_masks[(square / 8) + 7 - (square % 8)]) - (2 * bitwise::reverse(bin)));
			return (diag & diag_masks[(square / 8) + (square % 8)]) | (antidiag & antidiag_masks[(square / 8) + 7 - (square % 8)]);
		}
#endif

		inline bool is_square_attacked(uint8_t square, uint8_t by_who) const noexcept
		{
			if (pawn_attacks[!by_who][square] & this->state[side_to_piece_type[by_who][P]]) { return true; }

			if (knight_attacks[square] & this->state[side_to_piece_type[by_who][N]]) { return true; }
#ifdef _USE_MAGIC_BITBOARDS
			if (bishop_attacks_magic(square) & (this->state[side_to_piece_type[by_who][B]] | this->state[side_to_piece_type[by_who][Q]])) { return true; }

			if (rook_attacks_magic(square) & (this->state[side_to_piece_type[by_who][R]] | this->state[side_to_piece_type[by_who][Q]])) { return true; }
#else
			if (bishop_attacks_hq(square) & (this->state[side_to_piece_type[by_who][B]] | this->state[side_to_piece_type[by_who][Q]])) { return true; }

			if (rook_attacks_hq(square) & (this->state[side_to_piece_type[by_who][R]] | this->state[side_to_piece_type[by_who][Q]])) { return true; }
#endif
			if (king_attacks[square] & this->state[side_to_piece_type[by_who][K]]) { return true; }

			return false;
		}

		inline bool is_in_check() const noexcept
		{
			if (this->side == white)
			{
				uint64_t kbb = this->state[K];
				return is_square_attacked(bitwise::lsb(kbb), black);
			}
			else
			{
				uint64_t kbb = this->state[k];
				return is_square_attacked(bitwise::lsb(kbb), white);
			}
			return false;
		}

		inline bool is_repetition() const noexcept
		{
			if (!this->history.size()) { return false; }
			if (!this->fifty_move) { return false; }
			if (this->fifty_move > this->history.size()) { return false; }

			for (int i = this->history.size() - this->fifty_move; i < this->history.size(); ++i)
			{
				if (this->history[i].hashkey == this->hashkey) { return true; }
			}
			return false;
		}

		inline void remove_enpassant() noexcept
		{
			this->enpassant = no_sq;
		}

		inline void reset_killer_and_history_moves() noexcept
		{
			memset(this->killer_moves, 0, sizeof(this->killer_moves));
			memset(this->history_moves, 0, sizeof(this->history_moves));
		}

		inline void add_killer_move(uint32_t move, int depth) noexcept
		{
			this->killer_moves[1][depth] = this->killer_moves[0][depth];
			this->killer_moves[0][depth] = move;
		}

		inline void add_history_move(uint8_t score, uint8_t piece, uint8_t to_square) noexcept
		{
			this->history_moves[piece][to_square] = score;
		}

		inline uint8_t get_side() const noexcept
		{
			return this->side;
		}

		inline uint8_t get_fifty_move() const noexcept
		{
			return this->fifty_move;
		}

		inline void switch_side() noexcept
		{
			this->side ^= 1;
		}

		void generate_hashkey() noexcept;

		inline uint64_t get_hashkey() const noexcept
		{
			return this->hashkey;
		}

		uint8_t get_piece_score(int depth, uint8_t piece, uint8_t promoted_piece, uint8_t from_square, uint8_t to_square, bool is_capture, bool is_enpassant = false) noexcept;

		void generate_pseudolegal(move_list& moves, uint8_t type, int depth) noexcept;

		void generate_moves(move_list& moves, bool sort, uint8_t type, bool extract_legal, int depth) noexcept;

		inline void preserve_board(board_undo& undo_board) noexcept
		{
			memcpy(undo_board.state, this->state, sizeof(this->state));
			memcpy(undo_board.occupied, this->occupied, sizeof(this->occupied));
			undo_board.side = this->side;
			undo_board.castling = this->castling;
			undo_board.enpassant = this->enpassant;
			undo_board.fifty_move = this->fifty_move;
			undo_board.hashkey = this->hashkey;
		}

		inline void restore_board(const board_undo& undo_board) noexcept
		{
			memcpy(this->state, undo_board.state, sizeof(this->state));
			memcpy(this->occupied, undo_board.occupied, sizeof(this->occupied));
			this->side = undo_board.side;
			this->castling = undo_board.castling;
			this->enpassant = undo_board.enpassant;
			this->fifty_move = undo_board.fifty_move;
			this->hashkey = undo_board.hashkey;
		}

		inline bool push_history() noexcept
		{
			board_undo undo = { 0 };
			memcpy(undo.state, this->state, sizeof(this->state));
			memcpy(undo.occupied, this->occupied, sizeof(this->occupied));
			undo.side = this->side;
			undo.castling = this->castling;
			undo.enpassant = this->enpassant;
			undo.fifty_move = this->fifty_move;
			undo.hashkey = this->hashkey;
			this->history.push_back(undo);
			return true;
		}

		inline bool pop_history() noexcept
		{
			if (this->history.size() > 0)
			{
				board_undo undo = this->history[this->history.size() - 1];
				memcpy(this->state, undo.state, sizeof(this->state));
				memcpy(this->occupied, undo.occupied, sizeof(this->occupied));
				this->side = undo.side;
				this->castling = undo.castling;
				this->enpassant = undo.enpassant;
				this->fifty_move = undo.fifty_move;
				this->hashkey = undo.hashkey;
				this->history.pop_back();
			}
			return false;
		}

		inline uint32_t string_to_move(const std::string& move_str) noexcept
		{
			uint8_t ff = helper::letter_to_file(move_str[0]);
			uint8_t fr = 8 - (move_str[1] - '0');
			uint8_t tf = helper::letter_to_file(move_str[2]);
			uint8_t tr = 8 - (move_str[3] - '0');

			uint8_t move_from = helper::rank_and_file_to_square(fr, ff);
			uint8_t move_to = helper::rank_and_file_to_square(tr, tf);
			uint8_t move_promoted = 0;

			if (move_str.size() > 4)
			{
				switch (move_str[4])
				{
				case 'n':
					if (this->side == white) { move_promoted = N; }
					else { move_promoted = n; }
					break;
				case 'b':
					if (this->side == white) { move_promoted = B; }
					else { move_promoted = b; }
					break;
				case 'r':
					if (this->side == white) { move_promoted = R; }
					else { move_promoted = r; }
					break;
				case 'q':
					if (this->side == white) { move_promoted = Q; }
					else { move_promoted = q; }
					break;
				}
			}

			move_list moves{};
			generate_moves(moves, false, all_moves, false, 0);

			for (int i = 0; i < moves.m_size; ++i)
			{
				if (n_move::get_move_from(moves.m_moves[i]) == move_from && n_move::get_move_to(moves.m_moves[i]) == move_to && n_move::get_move_promoted_piece(moves.m_moves[i]) == move_promoted)
				{
					return moves.m_moves[i];
				}
			}
			return 0;
		}

		inline std::string move_to_string(uint32_t move) const noexcept
		{
			std::string move_str;

			uint8_t from = n_move::get_move_from(move);
			uint8_t to = n_move::get_move_to(move);
			uint8_t promoted_piece = n_move::get_move_promoted_piece(move);

			move_str += square_to_coords[from];
			move_str += square_to_coords[to];

			if (promoted_piece) { move_str += tolower(pieces_to_ascii[promoted_piece]); }

			return move_str;
		}

		bool make_move(uint32_t move, bool save_to_history) noexcept;

		inline int get_game_phase_score() const noexcept
		{
			int score = 0;

			for (uint8_t i = N; i <= Q; ++i) { score += (bitwise::count(this->state[i]) * material_evaluation[opening][i]); }

			for (uint8_t i = n; i <= q; ++i) { score += (bitwise::count(this->state[i]) * -material_evaluation[opening][i]); }

			return score;
		}

		inline uint64_t big_piece_exist() const noexcept
		{
			if (this->get_side() == white)
			{
				return (this->state[N] | this->state[B] | this->state[R] | this->state[Q]);
			}
			else
			{
				return (this->state[n] | this->state[b] | this->state[r] | this->state[q]);
			}
		}

		inline bool is_defended_by_pawns(uint8_t square, uint8_t by_who) const noexcept
		{
			if (pawn_attacks[!by_who][square] & this->state[side_to_piece_type[by_who][P]]) { return true; }
			return false;
		}

		inline uint8_t score_possible_pawn_attack(uint8_t attack_square) const noexcept
		{
			uint64_t attacks = pawn_attacks[this->side][attack_square];

			if (attacks & this->state[side_to_piece_type[!this->side][K]]) { return 65; } // pawn attacks king

			if (attacks & this->state[side_to_piece_type[!this->side][Q]]) { return 55; } // pawn attacks queen

			if (attacks & this->state[side_to_piece_type[!this->side][R]]) { return 45; } // pawn attacks rook

			if (attacks & this->state[side_to_piece_type[!this->side][B]]) { return 35; } // pawn attacks bishop

			if (attacks & this->state[side_to_piece_type[!this->side][N]]) { return 24; } // pawn attacks knight

			if (attacks & this->state[side_to_piece_type[!this->side][P]]) { return 15; } // pawn attacks pawn

			return 0;
		}

		inline uint8_t score_possible_knight_attack(uint8_t attack_square) const noexcept
		{
			uint64_t attacks = knight_attacks[attack_square];

			if (attacks & this->state[side_to_piece_type[!this->side][K]]) { return 64; } // knight attacks king

			if (attacks & this->state[side_to_piece_type[!this->side][Q]]) { return 54; } // knight attacks queen

			if (attacks & this->state[side_to_piece_type[!this->side][R]]) { return 44; } // knight attacks rook

			if (attacks & this->state[side_to_piece_type[!this->side][B]]) { return 34; } // knight attacks bishop

			if (attacks & this->state[side_to_piece_type[!this->side][N]]) { return 24; } // knight attacks knight

			if (attacks & this->state[side_to_piece_type[!this->side][P]]) { return 14; } // knight attacks pawn

			return 0;
		}

		inline uint8_t score_possible_bishop_attack(uint8_t attack_square) const noexcept
		{
#ifdef _USE_MAGIC_BITBOARDS
			uint64_t attacks = bishop_attacks_magic(attack_square);
#else
			uint64_t attacks = bishop_attacks_hq(attack_square);
#endif

			if (attacks & this->state[side_to_piece_type[!this->side][K]]) { return 63; } // bishop attacks king

			if (attacks & this->state[side_to_piece_type[!this->side][Q]]) { return 53; } // bishop attacks queen

			if (attacks & this->state[side_to_piece_type[!this->side][R]]) { return 43; } // bishop attacks rook

			if (attacks & this->state[side_to_piece_type[!this->side][B]]) { return 33; } // bishop attacks bishop

			if (attacks & this->state[side_to_piece_type[!this->side][N]]) { return 23; } // bishop attacks knight

			if (attacks & this->state[side_to_piece_type[!this->side][P]]) { return 13; } // bishop attacks pawn

			return 0;
		}

		inline uint8_t score_possible_rook_attack(uint8_t attack_square) const noexcept
		{
#ifdef _USE_MAGIC_BITBOARDS
			uint64_t attacks = rook_attacks_magic(attack_square);
#else
			uint64_t attacks = rook_attacks_hq(attack_square);
#endif

			if (attacks & this->state[side_to_piece_type[!this->side][K]]) { return 62; } // rook attacks king

			if (attacks & this->state[side_to_piece_type[!this->side][Q]]) { return 52; } // rook attacks queen

			if (attacks & this->state[side_to_piece_type[!this->side][R]]) { return 42; } // rook attacks rook

			if (attacks & this->state[side_to_piece_type[!this->side][B]]) { return 32; } // rook attacks bishop

			if (attacks & this->state[side_to_piece_type[!this->side][N]]) { return 22; } // rook attacks knight

			if (attacks & this->state[side_to_piece_type[!this->side][P]]) { return 12; } // rook attacks pawn

			return 0;
		}

		inline uint8_t score_possible_queen_attack(uint8_t attack_square) const noexcept
		{
#ifdef _USE_MAGIC_BITBOARDS
			uint64_t attacks = queen_attacks_magic(attack_square);
#else
			uint64_t attacks = rook_attacks_hq(attack_square) | bishop_attacks_hq(attack_square);
#endif

			if (attacks & this->state[side_to_piece_type[!this->side][K]]) { return 61; } // queen attacks king

			if (attacks & this->state[side_to_piece_type[!this->side][Q]]) { return 51; } // queen attacks queen

			if (attacks & this->state[side_to_piece_type[!this->side][R]]) { return 41; } // queen attacks rook

			if (attacks & this->state[side_to_piece_type[!this->side][B]]) { return 31; } // queen attacks bishop

			if (attacks & this->state[side_to_piece_type[!this->side][N]]) { return 21; } // queen attacks knight

			if (attacks & this->state[side_to_piece_type[!this->side][P]]) { return 11; } // queen attacks pawn

			return 0;
		}

		inline uint8_t score_possible_king_attack(uint8_t attack_square) const noexcept
		{
			uint64_t attacks = king_attacks[attack_square];

			if (attacks & this->state[side_to_piece_type[!this->side][R]]) { return 40; } // king attacks rook

			if (attacks & this->state[side_to_piece_type[!this->side][B]]) { return 30; } // king attacks bishop

			if (attacks & this->state[side_to_piece_type[!this->side][N]]) { return 20; } // king attacks knight

			if (attacks & this->state[side_to_piece_type[!this->side][P]]) { return 10; } // king attacks pawn

			return 0;
		}

		inline uint8_t score_possible_attack(uint8_t piece, uint8_t attack_square) const noexcept
		{
			if (piece == P) { return this->score_possible_pawn_attack(attack_square); }
			else if (piece == N) { return this->score_possible_knight_attack(attack_square); }
			else if (piece == B) { return this->score_possible_bishop_attack(attack_square); }
			else if (piece == R) { return this->score_possible_rook_attack(attack_square); }
			else if (piece == Q) { return this->score_possible_queen_attack(attack_square); }
			else if (piece == K) { return this->score_possible_king_attack(attack_square); }

			return 0;
		}

		int evaluate_norm() noexcept;

		int evaluate_nn() noexcept;

		int evaluate() noexcept;
};