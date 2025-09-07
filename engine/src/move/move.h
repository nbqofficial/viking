#pragma once

#include "../bitwise/bitwise.h"

namespace n_move
{
	inline constexpr uint32_t encode_move(uint8_t from, uint8_t to, uint8_t piece, uint8_t promo, uint8_t cap, uint8_t dbl, uint8_t ep, uint8_t castle, uint8_t score) noexcept
	{
		return  uint32_t(from) | (uint32_t(to) << 6) | (uint32_t(piece) << 12) | (uint32_t(promo) << 16) | (uint32_t(cap) << 20) | (uint32_t(dbl) << 21) | (uint32_t(ep) << 22) | (uint32_t(castle) << 23) | (uint32_t(score) << 24);
	}

	inline constexpr uint8_t get_move_from(uint32_t m) noexcept
	{
		return  m & 0x3F;
	}

	inline constexpr uint8_t get_move_to(uint32_t m) noexcept
	{
		return (m >> 6) & 0x3F;
	}

	inline constexpr uint8_t get_move_piece(uint32_t m) noexcept
	{
		return (m >> 12) & 0x0F;
	}

	inline constexpr uint8_t get_move_promoted_piece(uint32_t m) noexcept
	{
		return (m >> 16) & 0x0F;
	}

	inline constexpr uint8_t get_move_capture_flag(uint32_t m) noexcept
	{
		return (m >> 20) & 0x01;
	}

	inline constexpr uint8_t get_move_double_push_flag(uint32_t m) noexcept
	{
		return (m >> 21) & 0x01;
	}

	inline constexpr uint8_t get_move_enpassant_flag(uint32_t m) noexcept
	{
		return (m >> 22) & 0x01;
	}

	inline constexpr uint8_t get_move_castling_flag(uint32_t m) noexcept
	{
		return (m >> 23) & 0x01;
	}

	inline constexpr uint8_t get_move_score(uint32_t m) noexcept
	{
		return (m >> 24) & 0x3F;
	}
}