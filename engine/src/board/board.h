#pragma once

#include "../bitwise/bitwise.h"

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
		
		board();

		~board();

		bool reset();

		bool init(const std::string& fen, const bool& display_fen);

		void display();

		uint32_t encode_move(const uint8_t& from, const uint8_t& to, const uint8_t& piece, const uint8_t& promoted_piece, const uint8_t& capture_flag, const uint8_t& double_push_flag, const uint8_t& enpassant_flag, const uint8_t& castling_flag, const uint8_t& score);

		uint8_t get_move_from(const uint32_t& move);

		uint8_t get_move_to(const uint32_t& move);

		uint8_t get_move_piece(const uint32_t& move);

		uint8_t get_move_promoted_piece(const uint32_t& move);

		uint8_t get_move_capture_flag(const uint32_t& move);

		uint8_t get_move_double_push_flag(const uint32_t& move);

		uint8_t get_move_enpassant_flag(const uint32_t& move);

		uint8_t get_move_castling_flag(const uint32_t& move);

		uint8_t get_move_score(const uint32_t& move);

		void display_move(const uint32_t& move);

		void display_moves(const std::vector<uint32_t>& moves);

		void display_pv_debug(const std::vector<uint32_t>& pv, const int& depth);

		void display_info(const std::vector<uint32_t>& pv, const int& score, const int& depth, const long long& nodes);

		uint64_t rook_attacks(const uint8_t& square);

		uint64_t bishop_attacks(const uint8_t& square);

		bool is_square_attacked(const uint8_t& square, const uint8_t& by_who);

		bool is_in_check();

		bool is_repetition();

		void remove_enpassant();

		void reset_killer_and_history_moves();

		void add_killer_move(uint32_t move, const int& depth);

		void add_history_move(uint8_t score, const uint8_t& piece, const uint8_t& to_square);

		uint8_t get_side();

		uint8_t get_fifty_move();

		void switch_side();

		void generate_hashkey();

		uint64_t get_hashkey();

		uint8_t get_piece_score(const int& depth, const uint8_t& piece, const uint8_t promoted_piece, const uint8_t& from_square, const uint8_t& to_square, const bool& is_capture);

		void generate_moves(std::vector<uint32_t>& moves, const bool& sort, const uint8_t& type, const bool& extract_legal, const int& depth);

		std::vector<uint32_t> extract_legal_moves(std::vector<uint32_t> moves);

		void preserve_board(board_undo& undo_board);

		void restore_board(const board_undo& undo_board);

		bool push_history();

		bool pop_history();

		uint32_t string_to_move(const std::string& move_str);

		std::string move_to_string(const uint32_t& move);

		bool make_move(const uint32_t& move, const bool& save_to_history);

		int get_game_phase_score();

		int evaluate();
};