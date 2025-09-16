#ifndef NNUE_H
#define NNUE_H

#include <stdbool.h>
#include <stdalign.h>
#include <stdint.h>   // for int16_t
#include "misc.h"

/* Platform / linkage macros */
#if defined(_WIN32)
#define NNUE_CALL __cdecl
#if defined(NNUE_EXPORTS)
#define NNUE_API __declspec(dllexport)
#elif defined(NNUE_STATIC)
#define NNUE_API
#else
#define NNUE_API __declspec(dllimport)
#endif
#else
#define NNUE_CALL
#define NNUE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /* pieces */
    enum chessmen { blank, king, queen, rook, bishop, knight, pawn };

    /* Header must not introduce multiple external definitions.
       Give this table internal linkage (or move to .cpp with 'extern' here). */
    static const int pic_tab[14] = {
        blank, king, queen, rook, bishop, knight, pawn,
        king,  queen, rook,  bishop, knight, pawn,  blank
    };

#define PIECE(x)     (pic_tab[(x)])
#define COMBINE(c,x) ((x) + (c) * 6)

    /* nnue data */
    typedef struct {
        alignas(64) int16_t accumulation[2][256];
        bool        computedAccumulation;
    } Accumulator;

    typedef struct Position {
        int         player;    /* 0 = white, 1 = black */
        int* pieces;    /* sentinel-terminated list; pieces[i] uses BBC coding */
        int* squares;   /* squares[i] in 0..63 */
        Accumulator accumulator;
    } Position;

    /* Internal engine entry (C linkage so you can call it from C or C++) */
    int nnue_evaluate_pos(Position* pos);

    /* Public API (C linkage, consistent decorations) */
    NNUE_API void NNUE_CALL nnue_init(const char* evalFile);
    /* Evaluate a piece list */
    NNUE_API int  NNUE_CALL nnue_evaluate(int player, int* pieces, int* squares);
    /* Evaluate a FEN string directly */
    NNUE_API int  NNUE_CALL nnue_evaluate_fen(const char* fen);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NNUE_H */
