#pragma once

#include "../helper/helper.h"

namespace bitwise
{
	inline void set(uint64_t& bb, uint8_t n) noexcept
	{
		bb |= 1ULL << n;
	}

	inline void clear(uint64_t& bb, uint8_t n) noexcept
	{
		bb &= ~(1ULL << n);
	}

	inline void toggle(uint64_t& bb, uint8_t n) noexcept
	{
		bb ^= 1ULL << n;
	}

	inline bool check(uint64_t bb, uint8_t n) noexcept
	{
		return (bb >> n) & 1ULL;
	}

	inline uint8_t count(uint64_t bb) noexcept
	{
		return static_cast<uint8_t>(__popcnt64(bb));
	}

	inline int8_t lsb(uint64_t bb) noexcept
	{
		if (!bb) return -1;
		return static_cast<int8_t>(_tzcnt_u64(bb));
	}

	inline void negate(uint64_t& bb) noexcept
	{
		bb = ~bb;
	}

	inline uint64_t reverse(uint64_t bb) noexcept
	{
		bb = ((bb >> 1) & 0x5555555555555555ULL) | ((bb & 0x5555555555555555ULL) << 1);
		bb = ((bb >> 2) & 0x3333333333333333ULL) | ((bb & 0x3333333333333333ULL) << 2);
		bb = ((bb >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((bb & 0x0F0F0F0F0F0F0F0FULL) << 4);
		bb = ((bb >> 8) & 0x00FF00FF00FF00FFULL) | ((bb & 0x00FF00FF00FF00FFULL) << 8);
		bb = ((bb >> 16) & 0x0000FFFF0000FFFFULL) | ((bb & 0x0000FFFF0000FFFFULL) << 16);
		return (bb >> 32) | (bb << 32);
	}

	void display(const uint64_t& bitboard);
	
	void display_binary_move(const uint32_t& binary_move);
}