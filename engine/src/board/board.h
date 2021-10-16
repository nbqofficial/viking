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
		uint16_t ply;
		uint64_t hashkey;
		std::vector<board_undo> history;

	public:

		board();

		~board();

		bool reset();

		bool init(const std::string& fen);

		void display();

		uint32_t encode_move(const uint8_t& from, const uint8_t& to, const uint8_t& piece, const uint8_t& promoted_piece, const uint8_t& capture_flag, const uint8_t& double_push_flag, const uint8_t& enpassant_flag, const uint8_t& castling_flag, const uint8_t& mvvlva);

		uint8_t get_move_from(const uint32_t& move);

		uint8_t get_move_to(const uint32_t& move);

		uint8_t get_move_piece(const uint32_t& move);

		uint8_t get_move_promoted_piece(const uint32_t& move);

		uint8_t get_move_capture_flag(const uint32_t& move);

		uint8_t get_move_double_push_flag(const uint32_t& move);

		uint8_t get_move_enpassant_flag(const uint32_t& move);

		uint8_t get_move_castling_flag(const uint32_t& move);

		uint8_t get_move_mvvlva(const uint32_t& move);

		void display_move(const uint32_t& move);

		void display_moves(const std::vector<uint32_t>& moves);

		uint64_t rook_attacks(const uint8_t& square);

		uint64_t bishop_attacks(const uint8_t& square);

		bool is_square_attacked(const uint8_t& square, const uint8_t& by_who);

		uint8_t get_piece_mvvlva(const uint8_t& piece, const uint8_t& square);

		void generate_moves(std::vector<uint32_t>& moves, const bool& sort, const uint8_t& type);

		std::vector<uint32_t> extract_legal_moves(std::vector<uint32_t> moves);

		void save_history();

		bool make_move(const uint32_t& move);

		bool undo_move();
};