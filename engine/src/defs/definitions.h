#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>


#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"			// startpos
#define ruy_lopez "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 5 4"  // ruy lopez berlin defence castles
#define f6_enpassant_test "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3"	// f6 is en passant square
#define pawn_test "rnbqkb1r/p1p3P1/5n1p/1p1pp3/2PP1P2/8/PP4PP/RNBQKBNR w KQkq - 0 8"    // pawn captures, promotions, moves
#define wac1 "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1"				// Qg6


typedef struct _board_undo
{
    uint64_t state[12];
    uint64_t occupied[3];
    uint8_t side;
    uint8_t castling;
    uint8_t enpassant;
    uint8_t fifty_move;
    uint64_t hashkey;
}board_undo, *pboard_undo;


enum squares : uint8_t
{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum pieces : uint8_t
{
    P, N, B, R, Q, K, p, n, b, r, q, k
};

enum sides : uint8_t
{
    white, black, both
};

enum castle : uint8_t
{
    white_oo = 1, white_ooo = 2, black_oo = 4, black_ooo = 8
};


static const char* square_to_coords[] ={
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

static const char* pieces_to_ascii = "PNBRQKpnbrqk";

