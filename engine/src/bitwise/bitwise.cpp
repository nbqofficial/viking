#include "bitwise.h"

void bitwise::display(const uint64_t& bitboard)
{
    printf("\n");
    for (uint8_t r = 0; r < 8; ++r)
    {
        for (uint8_t f = 0; f < 8; ++f)
        {
            if (!f) { printf("\t%d  ", 8 - r); }
            uint8_t sq = r * 8 + f;
            printf("%d ", check(bitboard, sq));
        }
        printf("\n");
    }
    printf("\n\t   a b c d e f g h\n\n");
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
