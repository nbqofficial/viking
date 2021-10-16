#include "board.h"

board::board()
{
}

board::~board()
{
}

bool board::reset()
{
	memset(this->state, 0ULL, sizeof(this->state));
	memset(this->occupied, 0ULL, sizeof(this->occupied));
	this->side = white;	
	this->castling = 0;
	this->enpassant = no_sq;
	this->fifty_move = 0;
	this->ply = 0;
	this->hashkey = 0;
	this->history.clear();
	return true;
}

bool board::init(const std::string& fen)
{
	reset();

	uint16_t i = 0;
	uint16_t counter = 0;

	for (i = 0; i < fen.size(); ++i)
	{
		if (fen[i] == 'P')
		{
			bitwise::set(this->state[P], counter);
			counter++;
		}
		else if (fen[i] == 'N')
		{
			bitwise::set(this->state[N], counter);
			counter++;
		}
		else if (fen[i] == 'B')
		{
			bitwise::set(this->state[B], counter);
			counter++;
		}
		else if (fen[i] == 'R')
		{
			bitwise::set(this->state[R], counter);
			counter++;
		}
		else if (fen[i] == 'Q')
		{
			bitwise::set(this->state[Q], counter);
			counter++;
		}
		else if (fen[i] == 'K')
		{
			bitwise::set(this->state[K], counter);
			counter++;
		}
		else if (fen[i] == 'p')
		{
			bitwise::set(this->state[p], counter);
			counter++;
		}
		else if (fen[i] == 'n')
		{
			bitwise::set(this->state[n], counter);
			counter++;
		}
		else if (fen[i] == 'b')
		{
			bitwise::set(this->state[b], counter);
			counter++;
		}
		else if (fen[i] == 'r')
		{
			bitwise::set(this->state[r], counter);
			counter++;
		}
		else if (fen[i] == 'q')
		{
			bitwise::set(this->state[q], counter);
			counter++;
		}
		else if (fen[i] == 'k')
		{
			bitwise::set(this->state[k], counter);
			counter++;
		}
		else if (fen[i] == '1') 
		{
			counter++;
		}
		else if (fen[i] == '2')
		{
			for (uint8_t j = 0; j < 2; ++j) { counter++; }
		}
		else if (fen[i] == '3')
		{
			for (uint8_t j = 0; j < 3; ++j) { counter++; }
		}
		else if (fen[i] == '4')
		{
			for (uint8_t j = 0; j < 4; ++j) { counter++; }
		}
		else if (fen[i] == '5')
		{
			for (uint8_t j = 0; j < 5; ++j) { counter++; }
		}
		else if (fen[i] == '6')
		{
			for (uint8_t j = 0; j < 6; ++j) { counter++; }
		}
		else if (fen[i] == '7')
		{
			for (uint8_t j = 0; j < 7; ++j) { counter++; }
		}
		else if (fen[i] == '8')
		{
			for (uint8_t j = 0; j < 8; ++j) { counter++; }
		}
		else if (fen[i] == ' ')
		{
			i++;
			this->side = (fen[i] == 'w') ? white : black;

			i += 2;
			for (uint8_t j = 0; j < 4; ++j)
			{
				if (fen[i] == ' ') { break; }
				switch (fen[i])
				{
				case 'K': this->castling |= white_oo; break;
				case 'Q': this->castling |= white_ooo; break;
				case 'k': this->castling |= black_oo; break;
				case 'q': this->castling |= black_ooo; break;
				}
				i++;
			}
			i++;

			if (fen[i] != '-')
			{
				uint8_t file = helper::letter_to_file(fen[i]);
				uint8_t rank = 8 - (fen[i + 1] - '0');
				this->enpassant = helper::rank_and_file_to_square(rank, file);
			}
			break;
		}
	}

	for (uint8_t j = P; j <= K; ++j) { this->occupied[white] |= this->state[j]; }

	for (uint8_t j = p; j <= k; ++j) { this->occupied[black] |= this->state[j]; }

	this->occupied[both] |= this->occupied[white];
	this->occupied[both] |= this->occupied[black];

	return true;
}

void board::display()
{
	printf("\n\n");
	for (uint8_t r = 0; r < 8; ++r)
	{
		for (uint8_t f = 0; f < 8; ++f)
		{
			if (!f) { printf("\t\t\t\t%d  ", 8 - r); }
			
			int8_t piece = -1;
			uint8_t square = helper::rank_and_file_to_square(r, f);

			for (uint8_t i = P; i <= k; ++i)
			{
				if (bitwise::check(this->state[i], square))
				{
					piece = i;
					break;
				}
			}
			printf("%c ", (piece == -1) ? '.' : pieces_to_ascii[piece]);
		}
		printf("\n");
	}
	printf("\n\t\t\t\t   a b c d e f g h\n\n");

	printf("\t\t\t\tside:        %s\n", !this->side ? "white" : "black");

	printf("\t\t\t\tcastling:     %c%c%c%c\n", (this->castling & white_oo) ? 'K' : '-',
									     (this->castling & white_ooo) ? 'Q' : '-', 
							             (this->castling & black_oo) ? 'k' : '-', 
							             (this->castling & black_ooo) ? 'q' : '-');

	printf("\t\t\t\tenpassant:      %s\n\n", (this->enpassant != no_sq) ? square_to_coords[this->enpassant] : "--");
}

uint32_t board::encode_move(const uint8_t& from, const uint8_t& to, const uint8_t& piece, const uint8_t& promoted_piece, const uint8_t& capture_flag, const uint8_t& double_push_flag, const uint8_t& enpassant_flag, const uint8_t& castling_flag, const uint8_t& mvvlva)
{
	return ((from) | (to << 6) | (piece << 12) | (promoted_piece << 16) | (capture_flag << 20) | (double_push_flag << 21) | (enpassant_flag << 22) | (castling_flag << 23) | (mvvlva << 24));
}

uint8_t board::get_move_from(const uint32_t& move)
{
	return (move & 0x3f);
}

uint8_t board::get_move_to(const uint32_t& move)
{
	return ((move & 0xfc0) >> 6);
}

uint8_t board::get_move_piece(const uint32_t& move)
{
	return ((move & 0xf000) >> 12);
}

uint8_t board::get_move_promoted_piece(const uint32_t& move)
{
	return ((move & 0xf0000) >> 16);
}

uint8_t board::get_move_capture_flag(const uint32_t& move)
{
	return ((move & 0x100000) >> 20);
}

uint8_t board::get_move_double_push_flag(const uint32_t& move)
{
	return ((move & 0x200000) >> 21);
}

uint8_t board::get_move_enpassant_flag(const uint32_t& move)
{
	return ((move & 0x400000) >> 22);
}

uint8_t board::get_move_castling_flag(const uint32_t& move)
{
	return ((move & 0x800000) >> 23);
}

uint8_t board::get_move_mvvlva(const uint32_t& move)
{
	return ((move & 0x3f000000) >> 24);
}

void board::display_move(const uint32_t& move)
{
	uint8_t from = get_move_from(move);
	uint8_t to = get_move_to(move);
	uint8_t piece = get_move_piece(move);
	uint8_t promoted_piece = get_move_promoted_piece(move);
	uint8_t capture_flag = get_move_capture_flag(move);
	uint8_t double_push_flag = get_move_double_push_flag(move);
	uint8_t enpassant_flag = get_move_enpassant_flag(move);
	uint8_t castling_flag = get_move_castling_flag(move);
	uint8_t mvvlva = get_move_mvvlva(move);

	printf("\t%s%s%c", square_to_coords[from],
				       square_to_coords[to],
					   promoted_piece ? tolower(pieces_to_ascii[promoted_piece]) : ' ');

	printf("   %c", pieces_to_ascii[piece]);
	printf("     %c", promoted_piece ? pieces_to_ascii[promoted_piece] : '-');
	printf("        %d", capture_flag);
	printf("       %d", double_push_flag);
	printf("          %d", enpassant_flag);
	printf("         %d", castling_flag);
	printf("        %d\n", mvvlva);
}

void board::display_moves(const std::vector<uint32_t>& moves)
{
	printf("\tmove    piece promoted capture doublepush enpassant castling mvvlva\n\n");
	for (int i = 0; i < moves.size(); ++i)
	{
		display_move(moves[i]);
	}
	printf("\n\ttotal moves: %llu\n", moves.size());
}

uint64_t board::rook_attacks(const uint8_t& square)
{
	uint64_t bin = 1ULL << square;
	uint64_t hor = (this->occupied[both] - 2 * bin) ^ bitwise::reverse(bitwise::reverse(this->occupied[both]) - 2 * bitwise::reverse(bin));
	uint64_t ver = ((this->occupied[both] & file_masks[square % 8]) - (2 * bin)) ^ bitwise::reverse(bitwise::reverse(this->occupied[both] & file_masks[square % 8]) - (2 * bitwise::reverse(bin)));
	return (hor & rank_masks[square / 8]) | (ver & file_masks[square % 8]);
}

uint64_t board::bishop_attacks(const uint8_t& square)
{
	uint64_t bin = 1ULL << square;
	uint64_t diag = ((this->occupied[both] & diag_masks[(square / 8) + (square % 8)]) - (2 * bin)) ^ bitwise::reverse(bitwise::reverse(this->occupied[both] & diag_masks[(square / 8) + (square % 8)]) - (2 * bitwise::reverse(bin)));
	uint64_t antidiag = ((this->occupied[both] & antidiag_masks[(square / 8) + 7 - (square % 8)]) - (2 * bin)) ^ bitwise::reverse(bitwise::reverse(this->occupied[both] & antidiag_masks[(square / 8) + 7 - (square % 8)]) - (2 * bitwise::reverse(bin)));
	return (diag & diag_masks[(square / 8) + (square % 8)]) | (antidiag & antidiag_masks[(square / 8) + 7 - (square % 8)]);
}

bool board::is_square_attacked(const uint8_t& square, const uint8_t& by_who)
{
	if (pawn_attacks[!by_who][square] & this->state[side_to_piece_type[by_who][P]]) { return true; }
	
	if (knight_attacks[square] & this->state[side_to_piece_type[by_who][N]]) { return true; }

	if (bishop_attacks(square) & (this->state[side_to_piece_type[by_who][B]] | this->state[side_to_piece_type[by_who][Q]])) { return true; }

	if (rook_attacks(square) & (this->state[side_to_piece_type[by_who][R]] | this->state[side_to_piece_type[by_who][Q]])) { return true; }

	if (king_attacks[square] & this->state[side_to_piece_type[by_who][K]]) { return true; }

	return false;
}

uint8_t board::get_piece_mvvlva(const uint8_t& piece, const uint8_t& square)
{
	for (uint8_t i = P; i < K; ++i)
	{
		uint64_t bitboard = this->state[side_to_piece_type[!this->side][i]];

		if (bitwise::check(bitboard, square)) { return mvvlva[piece][i]; }
	}
	return 0;
}

void board::generate_moves(std::vector<uint32_t>& moves, const bool& sort)
{
	uint8_t from_square = 0;
	uint8_t to_square = 0;
	uint64_t bitboard = 0ULL;
	uint64_t attacks = 0ULL;

	if (this->side == white)
	{
		for (uint8_t piece = P; piece <= K; ++piece)
		{
			bitboard = this->state[piece];

			if (piece == P)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);
					to_square = from_square - 8;

					if (!(to_square < a8) && !bitwise::check(this->occupied[both], to_square))
					{
						if (from_square >= a7 && from_square <= h7)
						{
							moves.push_back(encode_move(from_square, to_square, piece, Q, 0, 0, 0, 0, mvvlva[K][P]));
							moves.push_back(encode_move(from_square, to_square, piece, R, 0, 0, 0, 0, mvvlva[K][P]));
							moves.push_back(encode_move(from_square, to_square, piece, B, 0, 0, 0, 0, mvvlva[K][P]));
							moves.push_back(encode_move(from_square, to_square, piece, N, 0, 0, 0, 0, mvvlva[K][P]));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						
							if ((from_square >= a2 && from_square <= h2) && !bitwise::check(this->occupied[both], to_square - 8))
							{
								moves.push_back(encode_move(from_square, to_square - 8, piece, 0, 0, 1, 0, 0, 0));
							}
						}
					}
			
					attacks = pawn_attacks[white][from_square] & this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (from_square >= a7 && from_square <= h7)
						{
							moves.push_back(encode_move(from_square, to_square, piece, Q, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
							moves.push_back(encode_move(from_square, to_square, piece, R, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
							moves.push_back(encode_move(from_square, to_square, piece, B, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
							moves.push_back(encode_move(from_square, to_square, piece, N, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					if (this->enpassant != no_sq)
					{
						uint64_t ep_attacks = pawn_attacks[white][from_square] & (1ULL << this->enpassant);

						if (ep_attacks)
						{
							uint8_t target_ep = bitwise::lsb(ep_attacks);
							moves.push_back(encode_move(from_square, target_ep, piece, 0, 1, 0, 1, 0, mvvlva[P][P]));
						}
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == N)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = knight_attacks[from_square] & ~this->occupied[white];
				
					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(N, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == B)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = bishop_attacks(from_square) & ~this->occupied[white];
				
					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(B, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == R)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = rook_attacks(from_square) & ~this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(R, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == Q)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = rook_attacks(from_square) | bishop_attacks(from_square);
					attacks &= ~this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(Q, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == K)
			{
				if (this->castling & white_oo)
				{
					if (!bitwise::check(this->occupied[both], f1) && !bitwise::check(this->occupied[both], g1))
					{
						if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
						{
							moves.push_back(encode_move(e1, g1, piece, 0, 0, 0, 0, 1, 0));
						}
					}
				}

				if (this->castling & white_ooo)
				{
					if (!bitwise::check(this->occupied[both], d1) && !bitwise::check(this->occupied[both], c1) && !bitwise::check(this->occupied[both], b1))
					{
						if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
						{
							moves.push_back(encode_move(e1, c1, piece, 0, 0, 0, 0, 1, 0));
						}
					}
				}

				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = king_attacks[from_square] & ~this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(K, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
		}
	}
	else
	{
		for (uint8_t piece = p; piece <= k; ++piece)
		{
			bitboard = this->state[piece];

			if (piece == p)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);
					to_square = from_square + 8;

					if (!(to_square > h1) && !bitwise::check(this->occupied[both], to_square))
					{
						if (from_square >= a2 && from_square <= h2)
						{
							moves.push_back(encode_move(from_square, to_square, piece, q, 0, 0, 0, 0, mvvlva[K][P]));
							moves.push_back(encode_move(from_square, to_square, piece, r, 0, 0, 0, 0, mvvlva[K][P]));
							moves.push_back(encode_move(from_square, to_square, piece, b, 0, 0, 0, 0, mvvlva[K][P]));
							moves.push_back(encode_move(from_square, to_square, piece, n, 0, 0, 0, 0, mvvlva[K][P]));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));

							if ((from_square >= a7 && from_square <= h7) && !bitwise::check(this->occupied[both], to_square + 8))
							{
								moves.push_back(encode_move(from_square, to_square + 8, piece, 0, 0, 1, 0, 0, 0));
							}
						}
					}

					attacks = pawn_attacks[black][from_square] & this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (from_square >= a2 && from_square <= h2)
						{
							moves.push_back(encode_move(from_square, to_square, piece, q, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
							moves.push_back(encode_move(from_square, to_square, piece, r, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
							moves.push_back(encode_move(from_square, to_square, piece, b, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
							moves.push_back(encode_move(from_square, to_square, piece, n, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(P, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					if (this->enpassant != no_sq)
					{
						uint64_t ep_attacks = pawn_attacks[black][from_square] & (1ULL << this->enpassant);

						if (ep_attacks)
						{
							uint8_t target_ep = bitwise::lsb(ep_attacks);
							moves.push_back(encode_move(from_square, target_ep, piece, 0, 1, 0, 1, 0, mvvlva[P][P]));
						}
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == n)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = knight_attacks[from_square] & ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(N, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == b)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = bishop_attacks(from_square) & ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(B, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == r)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = rook_attacks(from_square) & ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(R, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == q)
			{
				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = rook_attacks(from_square) | bishop_attacks(from_square);
					attacks &= ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(Q, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == k)
			{
				if (this->castling & black_oo)
				{
					if (!bitwise::check(this->occupied[both], f8) && !bitwise::check(this->occupied[both], g8))
					{
						if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
						{
							moves.push_back(encode_move(e8, g8, piece, 0, 0, 0, 0, 1, 0));
						}
					}
				}

				if (this->castling & black_ooo)
				{
					if (!bitwise::check(this->occupied[both], d8) && !bitwise::check(this->occupied[both], c8) && !bitwise::check(this->occupied[both], b8))
					{
						if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
						{
							moves.push_back(encode_move(e8, c8, piece, 0, 0, 0, 0, 1, 0));
						}
					}
				}

				while (bitboard)
				{
					from_square = bitwise::lsb(bitboard);

					attacks = king_attacks[from_square] & ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, 0));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_mvvlva(K, to_square)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
		}
	}

	if (sort)
	{
		if (sort) { std::sort(moves.begin(), moves.end(), [](const uint32_t& a, const uint32_t& b) { return ((a & 0x3f000000) >> 24) > ((b & 0x3f000000) >> 24); }); }
	}
}

void board::save_history()
{
	board_undo undo = { 0 };
	memcpy(undo.state, this->state, sizeof(this->state));
	memcpy(undo.occupied, this->occupied, sizeof(this->occupied));
	undo.side = this->side;
	undo.castling = this->castling;
	undo.enpassant = this->enpassant;
	undo.fifty_move = this->fifty_move;
	undo.hashkey = this->hashkey;

	this->history.push_back(undo);
	this->ply++;
}

bool board::make_move(uint32_t move, const uint8_t& type)
{
	
}

bool board::undo_move()
{
	if (this->history.size() > 0)
	{
		board_undo undo = this->history[this->history.size() - 1];
		memcpy(this->state, undo.state, sizeof(this->state));
		memcpy(this->occupied, undo.occupied, sizeof(this->occupied));
		this->side = undo.side;
		this->castling = undo.castling;
		this->enpassant = undo.enpassant;
		this->fifty_move = undo.fifty_move;
		this->hashkey = undo.hashkey;

		this->history.pop_back();
		this->ply--;

		return true;
	}
	return false;
}

