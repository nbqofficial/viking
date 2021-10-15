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

		bool init(const std::string& fen, const bool& display_fen);

		void display();
};