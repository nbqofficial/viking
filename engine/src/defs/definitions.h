#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>


#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"			        // startpos
#define four_knights "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 4 4"         // four knights opening
#define queen_h5_opening "r1bqkbnr/pppp1ppp/2n5/4p2Q/4P3/8/PPPP1PPP/RNB1KBNR w KQkq - 2 3"          // e4 e5 Qh5 Nc6
#define scandinavian_defense "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"         // e4 d5 scandi
#define ruy_lopez "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 5 4"              // ruy lopez berlin defence castles
#define italian_fried_liver "r1bqkb1r/ppp2ppp/2n2n2/3pp1N1/2B1P3/8/PPPP1PPP/RNBQK2R w KQkq - 0 5"   // italian game fried liver attack
#define f6_enpassant_test "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3"	        // f6 is en passant square
#define pawn_test "rnbqkb1r/p1p3P1/5n1p/1p1pp3/2PP1P2/8/PP4PP/RNBQKBNR w KQkq - 0 8"                // pawn captures, promotions, moves
#define wac1 "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1"				            // Qg6


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

static const uint8_t side_to_piece_type[2][6] = {
    { P, N, B, R, Q, K }, { p, n, b, r, q, k } };

static const uint64_t file_a = 72340172838076673L;

static const uint64_t file_h = -9187201950435737472L;

static const uint64_t file_ab = 217020518514230019L;

static const uint64_t file_gh = -4557430888798830400L;

static const uint64_t rank_1 = -72057594037927936L;

static const uint64_t rank_4 = 1095216660480L;

static const uint64_t rank_5 = 4278190080L;

static const uint64_t rank_8 = 255L;

static const uint64_t center = 103481868288L;

static const uint64_t extended_center = 66229406269440L;

static const uint64_t king_side = -1085102592571150096L;

static const uint64_t queen_side = 1085102592571150095L;

static const uint64_t rank_masks[] = { 0xFFL, 0xFF00L, 0xFF0000L, 0xFF000000L, 0xFF00000000L, 0xFF0000000000L, 0xFF000000000000L, 0xFF00000000000000L };

static const uint64_t file_masks[] = { 0x101010101010101L, 0x202020202020202L, 0x404040404040404L, 0x808080808080808L, 0x1010101010101010L, 0x2020202020202020L, 0x4040404040404040L, 0x8080808080808080L };

static const uint64_t diag_masks[] = { 0x1L, 0x102L, 0x10204L, 0x1020408L, 0x102040810L, 0x10204081020L, 0x1020408102040L, 0x102040810204080L, 0x204081020408000L, 0x408102040800000L, 0x810204080000000L, 0x1020408000000000L, 0x2040800000000000L, 0x4080000000000000L, 0x8000000000000000L };

static const uint64_t antidiag_masks[] = { 0x80L, 0x8040L, 0x804020L, 0x80402010L, 0x8040201008L, 0x804020100804L, 0x80402010080402L, 0x8040201008040201L, 0x4020100804020100L, 0x2010080402010000L, 0x1008040201000000L, 0x804020100000000L, 0x402010000000000L, 0x201000000000000L, 0x100000000000000L };

static const uint64_t knight_attacks[] = {
132096, 329728, 659712, 1319424, 2638848, 5277696, 10489856, 4202496,
33816580, 84410376, 168886289, 337772578, 675545156, 1351090312, 2685403152, 
1075839008, 8657044482, 21609056261, 43234889994, 86469779988, 172939559976, 
345879119952, 687463207072, 275414786112, 2216203387392, 5531918402816, 11068131838464, 
22136263676928, 44272527353856, 88545054707712, 175990581010432, 70506185244672,
567348067172352, 1416171111120896, 2833441750646784, 5666883501293568, 11333767002587136, 
22667534005174272, 45053588738670592, 18049583422636032, 145241105196122112, 362539804446949376,
725361088165576704, 1450722176331153408, 2901444352662306816, 5802888705324613632, 11533718717099671552,
4620693356194824192, 288234782788157440, 576469569871282176, 1224997833292120064, 2449995666584240128, 
4899991333168480256, 9799982666336960512, 1152939783987658752, 2305878468463689728, 1128098930098176, 
2257297371824128, 4796069720358912, 9592139440717824, 19184278881435648, 
38368557762871296, 4679521487814656, 9077567998918656 };

static const uint64_t king_attacks[] = {
770, 1797, 3594, 7188, 14376, 28752, 57504, 49216, 197123, 460039, 920078,
1840156, 3680312, 7360624, 14721248, 12599488, 50463488, 117769984, 235539968, 
471079936, 942159872, 1884319744, 3768639488, 3225468928, 12918652928, 30149115904,
60298231808, 120596463616, 241192927232, 482385854464, 964771708928, 825720045568, 
3307175149568, 7718173671424, 15436347342848, 30872694685696, 61745389371392, 123490778742784,
246981557485568, 211384331665408, 846636838289408, 1975852459884544, 3951704919769088, 
7903409839538176, 15806819679076352, 31613639358152704, 63227278716305408, 54114388906344448,
216739030602088448, 505818229730443264, 1011636459460886528, 2023272918921773056, 4046545837843546112,
8093091675687092224, 16186183351374184448, 13853283560024178688, 144959613005987840, 362258295026614272,
724516590053228544, 1449033180106457088, 2898066360212914176, 5796132720425828352, 
11592265440851656704, 4665729213955833856 };

static const uint64_t pawn_attacks[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 10, 20, 40, 80, 160, 64, 512, 1280, 2560, 5120, 10240, 20480, 40960, 16384,
      131072, 327680, 655360, 1310720, 2621440, 5242880, 10485760, 4194304, 33554432, 83886080, 167772160, 
      335544320, 671088640, 1342177280, 2684354560, 1073741824, 8589934592, 21474836480, 42949672960, 85899345920, 
      171798691840, 343597383680, 687194767360, 274877906944, 2199023255552, 5497558138880, 10995116277760, 21990232555520,
      43980465111040, 87960930222080, 175921860444160, 70368744177664, 562949953421312, 1407374883553280, 2814749767106560,
      5629499534213120, 11258999068426240, 22517998136852480, 45035996273704960, 18014398509481984 },
    {
      512, 1280, 2560, 5120, 10240, 20480, 40960, 16384, 131072, 327680, 655360, 1310720, 2621440, 5242880, 10485760, 
      4194304, 33554432, 83886080, 167772160, 335544320, 671088640, 1342177280, 2684354560, 1073741824, 8589934592, 
      21474836480, 42949672960, 85899345920, 171798691840, 343597383680, 687194767360, 274877906944, 2199023255552, 
      5497558138880, 10995116277760, 21990232555520, 43980465111040, 87960930222080, 175921860444160, 70368744177664,
      562949953421312, 1407374883553280, 2814749767106560, 5629499534213120, 11258999068426240, 22517998136852480, 
      45035996273704960, 18014398509481984, 144115188075855872, 360287970189639680, 720575940379279360, 1441151880758558720, 
      2882303761517117440, 5764607523034234880, 11529215046068469760, 4611686018427387904, 0, 0, 0, 0, 0, 0, 0, 0, }
};

static const uint8_t mvvlva[6][5] = {
    { 15, 25, 35, 45, 55 },
    { 14, 24, 34, 44, 54 },
    { 13, 23, 33, 43, 53 },
    { 12, 22, 32, 42, 52 },
    { 11, 21, 31, 41, 51 },
    { 10, 20, 30, 40, 50}
};


// move representation
// 0000 0000 0000 0000 0000 0000 0011 1111      from                0x3f
// 0000 0000 0000 0000 0000 1111 1100 0000      to                  0xfc0
// 0000 0000 0000 0000 1111 0000 0000 0000      piece               0xf000
// 0000 0000 0000 1111 0000 0000 0000 0000      promoted piece      0xf0000
// 0000 0000 0001 0000 0000 0000 0000 0000      capture flag        0x100000
// 0000 0000 0010 0000 0000 0000 0000 0000      double push flag    0x200000
// 0000 0000 0100 0000 0000 0000 0000 0000      enpassant flag      0x400000
// 0000 0000 1000 0000 0000 0000 0000 0000      castling flag       0x800000
// 0011 1111 0000 0000 0000 0000 0000 0000      mvvlva score        0x3f000000 (mvvlva 0 - 63)