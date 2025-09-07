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
		uint8_t counter = 0;
		while (bb)
		{
			counter++;
			bb &= bb - 1;
		}
		return counter;
	}

	inline int8_t lsb(uint64_t bb) noexcept
	{
		if (bb) { return count((bb & -(int64_t)bb) - 1); }
		else { return -1; }
	}

	inline void negate(uint64_t& bb) noexcept
	{
		bb = ~bb;
	}

	inline uint64_t reverse(uint64_t bb) noexcept
	{
		uint64_t result = bb;
		result = ((result >> 1) & ((uint64_t)0x55555555 << 32 | 0x55555555)) | ((result << 1) & ((uint64_t)0xaaaaaaaa << 32 | 0xaaaaaaaa));
		result = ((result >> 2) & ((uint64_t)0x33333333 << 32 | 0x33333333)) | ((result << 2) & ((uint64_t)0xcccccccc << 32 | 0xcccccccc));
		result = ((result >> 4) & ((uint64_t)0x0f0f0f0f << 32 | 0x0f0f0f0f)) | ((result << 4) & ((uint64_t)0xf0f0f0f0 << 32 | 0xf0f0f0f0));
		result = ((result >> 8) & ((uint64_t)0x00ff00ff << 32 | 0x00ff00ff)) | ((result << 8) & ((uint64_t)0xff00ff00 << 32 | 0xff00ff00));
		result = ((result >> 16) & ((uint64_t)0x0000ffff << 32 | 0x0000ffff)) | ((result << 16) & ((uint64_t)0xffff0000 << 32 | 0xffff0000));
		result = (result >> 32) | (result << 32);
		return result;
	}

	void display(const uint64_t& bitboard);
	
	void display_binary_move(const uint32_t& binary_move);
}