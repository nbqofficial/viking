#include "../src/uci/uci.h"

#ifdef _USE_MAGIC_BITBOARDS
namespace init
{
    uint64_t mask_rook_attacks(uint8_t square) noexcept
    {
        uint64_t attacks = 0ULL;

        int r, f;
        int tr = square / 8;
        int tf = square % 8;

        for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
        for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
        for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
        for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

        return attacks;
    }

    uint64_t mask_bishop_attacks(uint8_t square) noexcept
    {
        uint64_t attacks = 0ULL;

        int r, f;
        int tr = square / 8;
        int tf = square % 8;

        for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
        for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
        for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
        for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

        return attacks;
    }

    uint64_t set_occupancy(int index, int bits_in_mask, uint64_t attack_mask) noexcept
    {
        uint64_t occupancy = 0ULL;

        for (int count = 0; count < bits_in_mask; count++)
        {
            int square = bitwise::lsb(attack_mask);

            bitwise::clear(attack_mask, square);

            if (index & (1 << count))
                occupancy |= (1ULL << square);
        }

        return occupancy;
    }

    uint64_t bishop_attacks_on_the_fly(int square, uint64_t block) noexcept
    {
        uint64_t attacks = 0ULL;

        int r, f;
        int tr = square / 8;
        int tf = square % 8;

        for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
        {
            attacks |= (1ULL << (r * 8 + f));
            if ((1ULL << (r * 8 + f)) & block) break;
        }

        return attacks;
    }

    uint64_t rook_attacks_on_the_fly(int square, uint64_t block) noexcept
    {
        uint64_t attacks = 0ULL;

        int r, f;

        int tr = square / 8;
        int tf = square % 8;

        for (r = tr + 1; r <= 7; r++)
        {
            attacks |= (1ULL << (r * 8 + tf));
            if ((1ULL << (r * 8 + tf)) & block) break;
        }

        for (r = tr - 1; r >= 0; r--)
        {
            attacks |= (1ULL << (r * 8 + tf));
            if ((1ULL << (r * 8 + tf)) & block) break;
        }

        for (f = tf + 1; f <= 7; f++)
        {
            attacks |= (1ULL << (tr * 8 + f));
            if ((1ULL << (tr * 8 + f)) & block) break;
        }

        for (f = tf - 1; f >= 0; f--)
        {
            attacks |= (1ULL << (tr * 8 + f));
            if ((1ULL << (tr * 8 + f)) & block) break;
        }

        return attacks;
    }

    void init_sliders_attacks(bool is_bishop) noexcept
    {
        for (int square = 0; square < 64; square++)
        {
            bishop_masks[square] = mask_bishop_attacks(square);
            rook_masks[square] = mask_rook_attacks(square);

            uint64_t attack_mask = is_bishop ? bishop_masks[square] : rook_masks[square];
            int relevant_bits_count = bitwise::count(attack_mask);
            int occupancy_indices = (1 << relevant_bits_count);

            for (int index = 0; index < occupancy_indices; index++)
            {
                if (is_bishop)
                {
                    uint64_t occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                    int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                    bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
                }
                else
                {
                    uint64_t occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                    int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                    rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);

                }
            }
        }
    }

    void init_all() noexcept
    {
        init_sliders_attacks(false);
        init_sliders_attacks(true);
    }
}
#endif

int main()
{
#ifdef _USE_MAGIC_BITBOARDS
    init::init_all();
#endif
	srand(time(0));

	uci u;
	u.uci_loop();

	return 0;
}