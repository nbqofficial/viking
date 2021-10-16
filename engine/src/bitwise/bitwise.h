#pragma once

#include "../helper/helper.h"

namespace bitwise
{
	void set(uint64_t& bitboard, const uint8_t& n);

	void clear(uint64_t& bitboard, const uint8_t& n);

	void toggle(uint64_t& bitboard, const uint8_t& n);

	bool check(uint64_t bitboard, const uint8_t& n);

	uint16_t count(uint64_t bitboard);

	int16_t lsb(uint64_t bitboard);

	void negate(uint64_t& bitboard);

	uint64_t reverse(uint64_t bitboard);

	void display(const uint64_t& bitboard);
}