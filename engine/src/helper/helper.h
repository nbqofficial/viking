#pragma once

#include "../defs/definitions.h"

namespace helper
{
	uint8_t letter_to_file(const char& letter);

	char file_to_letter(const uint8_t& file);

	uint8_t rank_and_file_to_square(const uint8_t& rank, const uint8_t& file);
}