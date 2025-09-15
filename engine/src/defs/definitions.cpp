#include "definitions.h"

#ifdef _USE_MAGIC_BITBOARDS
uint64_t rook_masks[64];

uint64_t bishop_masks[64];

uint64_t rook_attacks[64][4096];

uint64_t bishop_attacks[64][512];
#endif