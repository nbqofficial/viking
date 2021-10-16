#include "bitwise.h"

void bitwise::set(uint64_t& bitboard, const uint8_t& n)
{
	bitboard |= 1ULL << n;
}

void bitwise::clear(uint64_t& bitboard, const uint8_t& n)
{
	bitboard &= ~(1ULL << n);
}

void bitwise::toggle(uint64_t& bitboard, const uint8_t& n)
{
	bitboard ^= 1ULL << n;
}

bool bitwise::check(uint64_t bitboard, const uint8_t& n)
{
	return (bitboard >> n) & 1ULL;
}

uint16_t bitwise::count(uint64_t bitboard)
{
    uint8_t counter = 0;

    while (bitboard)
    {
        counter++;
        bitboard &= bitboard - 1;
    }
    return counter;
}

int16_t bitwise::lsb(uint64_t bitboard)
{
    if (bitboard) 
    { 
        return count((bitboard & -(int64_t)bitboard) - 1);
    }
    else { return -1; }
}

void bitwise::negate(uint64_t& bitboard)
{
	bitboard = ~bitboard;
}

uint64_t bitwise::reverse(uint64_t bitboard)
{
    uint64_t result = bitboard;
    result = ((result >> 1) & ((uint64_t)0x55555555 << 32 | 0x55555555)) | ((result << 1) & ((uint64_t)0xaaaaaaaa << 32 | 0xaaaaaaaa));
    result = ((result >> 2) & ((uint64_t)0x33333333 << 32 | 0x33333333)) | ((result << 2) & ((uint64_t)0xcccccccc << 32 | 0xcccccccc));
    result = ((result >> 4) & ((uint64_t)0x0f0f0f0f << 32 | 0x0f0f0f0f)) | ((result << 4) & ((uint64_t)0xf0f0f0f0 << 32 | 0xf0f0f0f0));
    result = ((result >> 8) & ((uint64_t)0x00ff00ff << 32 | 0x00ff00ff)) | ((result << 8) & ((uint64_t)0xff00ff00 << 32 | 0xff00ff00));
    result = ((result >> 16) & ((uint64_t)0x0000ffff << 32 | 0x0000ffff)) | ((result << 16) & ((uint64_t)0xffff0000 << 32 | 0xffff0000));
    result = (result >> 32) | (result << 32);
    return result;
}

void bitwise::display(const uint64_t& bitboard)
{
    printf("\n\n");
    for (uint8_t r = 0; r < 8; ++r)
    {
        for (uint8_t f = 0; f < 8; ++f)
        {
            if (!f) { printf("\t\t\t\t%d  ", 8 - r); }
            uint8_t sq = r * 8 + f;
            printf("%d ", check(bitboard, sq));
        }
        printf("\n");
    }
    printf("\n\t\t\t\t   a b c d e f g h\n\n");
}

void bitwise::display_binary_move(const uint32_t& binary_move)
{
    for (uint8_t i = 0; i < 64; ++i)
    {
        if (i % 4 == 0) { printf(" "); }
        printf("%d", check(binary_move, i));
    }
    printf("\n");
}
