#include "helper.h"

uint8_t helper::letter_to_file(const char& letter)
{
    return (letter - '0') - 49;
}

char helper::file_to_letter(const uint8_t& file)
{
    return (file + '0') + 49;
}

uint8_t helper::rank_and_file_to_square(const uint8_t& rank, const uint8_t& file)
{
    return (rank * 8 + file);
}