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
	this->hashkey = 0;
	this->history.clear();
	memset(this->history_moves, 0, sizeof(this->history_moves));
	this->pv_line.clear();
	return true;
}

bool board::init(const std::string& fen, const bool& display_fen)
{
	if (display_fen) { printf("\tfen: %s\n", fen.c_str()); }

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

	generate_hashkey();

	return true;
}

void board::display()
{
	printf("\n");
	for (uint8_t r = 0; r < 8; ++r)
	{
		for (uint8_t f = 0; f < 8; ++f)
		{
			if (!f) { printf("\t%d  ", 8 - r); }
			
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
	printf("\n\t   a b c d e f g h\n\n");

	printf("\tside:        %s\n", !this->side ? "white" : "black");

	printf("\tcastling:     %c%c%c%c\n", (this->castling & white_oo) ? 'K' : '-',
									     (this->castling & white_ooo) ? 'Q' : '-', 
							             (this->castling & black_oo) ? 'k' : '-', 
							             (this->castling & black_ooo) ? 'q' : '-');

	printf("\tenpassant:      %s\n", (this->enpassant != no_sq) ? square_to_coords[this->enpassant] : "--");
	printf("\tfifty moves:\t%d\n", this->fifty_move);
	printf("\thistory:\t%llu\n", this->history.size());
	printf("\t %llx\n\n", this->hashkey);
}

uint32_t board::encode_move(const uint8_t& from, const uint8_t& to, const uint8_t& piece, const uint8_t& promoted_piece, const uint8_t& capture_flag, const uint8_t& double_push_flag, const uint8_t& enpassant_flag, const uint8_t& castling_flag, const uint8_t& score)
{
	return ((from) | (to << 6) | (piece << 12) | (promoted_piece << 16) | (capture_flag << 20) | (double_push_flag << 21) | (enpassant_flag << 22) | (castling_flag << 23) | (score << 24));
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

uint8_t board::get_move_score(const uint32_t& move)
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
	uint8_t mvvlva = get_move_score(move);

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
	printf("\tmove    piece promoted capture doublepush enpassant castling score\n\n");
	for (int i = 0; i < moves.size(); ++i)
	{
		display_move(moves[i]);
	}
	printf("\n\ttotal moves: %llu\n", moves.size());
}

void board::display_pv_debug(const std::vector<uint32_t>& pv, const int& depth)
{
	printf("\t%d          ", depth);
	for (int i = 0; i < pv.size(); ++i)
	{
		uint8_t from = get_move_from(pv[i]);
		uint8_t to = get_move_to(pv[i]);
		uint8_t promoted_piece = get_move_promoted_piece(pv[i]);

		printf("%s%s%c ", square_to_coords[from],
						  square_to_coords[to],
						  promoted_piece ? tolower(pieces_to_ascii[promoted_piece]) : ' ');
	}
	printf("\n");
}

void board::display_info(const std::vector<uint32_t>& pv, const int& score, const int& depth, const long long& nodes)
{
	printf("info score cp %d depth %d nodes %lld pv ", score, depth, nodes);

	for (int i = 0; i < pv.size(); ++i)
	{
		uint8_t from = get_move_from(pv[i]);
		uint8_t to = get_move_to(pv[i]);
		uint8_t promoted_piece = get_move_promoted_piece(pv[i]);

		if (promoted_piece)
		{
			printf("%s%s%c ", square_to_coords[from],
				              square_to_coords[to],
				              tolower(pieces_to_ascii[promoted_piece]));
		}
		else
		{
			printf("%s%s ", square_to_coords[from],
						    square_to_coords[to]);
		}
	}
	printf("\n");
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

bool board::is_in_check()
{
	if (this->side == white)
	{
		uint64_t kbb = this->state[K];
		return is_square_attacked(bitwise::lsb(kbb), black);
	}
	else
	{
		uint64_t kbb = this->state[k];
		return is_square_attacked(bitwise::lsb(kbb), white);
	}
	return false;
}

bool board::is_repetition()
{
	if (!this->history.size()) { return false; }
	if (!this->fifty_move) { return false; }
	if (this->fifty_move > this->history.size()) { return false; }

	for (int i = this->history.size() - this->fifty_move; i < this->history.size(); ++i)
	{
		if (this->history[i].hashkey == this->hashkey) { return true; }
	}
	return false;
}

void board::remove_enpassant()
{
	this->enpassant = no_sq;
}

void board::reset_killer_and_history_moves()
{
	memset(this->killer_moves, 0, sizeof(this->killer_moves));
	memset(this->history_moves, 0, sizeof(this->history_moves));
}

void board::add_killer_move(uint32_t move, const int& depth)
{
	this->killer_moves[1][depth] = this->killer_moves[0][depth];
	this->killer_moves[0][depth] = move;
}

void board::add_history_move(uint8_t score, const uint8_t& piece, const uint8_t& to_square)
{
	this->history_moves[piece][to_square] = score;
}

uint8_t board::get_side()
{
	return this->side;
}

uint8_t board::get_fifty_move()
{
	return this->fifty_move;
}

void board::switch_side()
{
	this->side ^= 1;
}

void board::generate_hashkey()
{
	this->hashkey = 0ULL;	
	for (uint8_t i = P; i <= k; ++i) 
	{ 
		uint64_t bitboard = this->state[i];
		while (bitboard)
		{
			uint8_t square = bitwise::lsb(bitboard);

			this->hashkey ^= state_hashkey[i][square];

			bitwise::clear(bitboard, square);
		}
	}
	if (this->enpassant != no_sq) { this->hashkey ^= enpassant_hashkey[this->enpassant]; }
	this->hashkey ^= castling_hashkey[this->castling];
	if (this->side == black) { this->hashkey ^= side_hashkey; }
}

uint64_t board::get_hashkey()
{
	return this->hashkey;
}

uint8_t board::get_piece_score(const int& depth, const uint8_t& piece, const uint8_t promoted_piece, const uint8_t& from_square, const uint8_t& to_square, const bool& is_capture)
{
	if (this->pv_line.size() && get_move_from(this->pv_line[depth]) == from_square && get_move_to(this->pv_line[depth]) == to_square && get_move_promoted_piece(this->pv_line[depth]) == promoted_piece)
	{
		return 100;
	}
	if (is_capture)
	{
		for (uint8_t i = P; i < K; ++i)
		{
			uint64_t bitboard = this->state[side_to_piece_type[!this->side][i]];
			if (bitwise::check(bitboard, to_square)) { return mvvlva[piece][i]; }
		}
	}
	else
	{
		if (get_move_from(this->killer_moves[0][depth]) == from_square && get_move_to(this->killer_moves[0][depth]) == to_square && get_move_promoted_piece(this->killer_moves[0][depth]) == promoted_piece)
		{
			return 9;
		}
		else if (get_move_from(this->killer_moves[1][depth]) == from_square && get_move_to(this->killer_moves[1][depth]) == to_square && get_move_promoted_piece(this->killer_moves[1][depth]) == promoted_piece)
		{
			return 8;
		}
		else
		{
			return this->history_moves[side_to_piece_type[this->side][piece]][to_square];
		}
	}
	return 0;
}

void board::generate_moves(std::vector<uint32_t>& moves, const bool& sort, const uint8_t& type, const bool& extract_legal, const int& depth)
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
							moves.push_back(encode_move(from_square, to_square, piece, Q, 0, 0, 0, 0, get_piece_score(depth, P, Q, from_square, to_square, false)));
							moves.push_back(encode_move(from_square, to_square, piece, R, 0, 0, 0, 0, get_piece_score(depth, P, R, from_square, to_square, false)));
							moves.push_back(encode_move(from_square, to_square, piece, B, 0, 0, 0, 0, get_piece_score(depth, P, B, from_square, to_square, false)));
							moves.push_back(encode_move(from_square, to_square, piece, N, 0, 0, 0, 0, get_piece_score(depth, P, N, from_square, to_square, false)));
						}
						else
						{
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, false)));

								if ((from_square >= a2 && from_square <= h2) && !bitwise::check(this->occupied[both], to_square - 8))
								{
									moves.push_back(encode_move(from_square, to_square - 8, piece, 0, 0, 1, 0, 0, get_piece_score(depth, P, 0, from_square, to_square - 8, false)));
								}
							}
						}
					}
			
					attacks = pawn_attacks[white][from_square] & this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (from_square >= a7 && from_square <= h7)
						{
							moves.push_back(encode_move(from_square, to_square, piece, Q, 1, 0, 0, 0, get_piece_score(depth, P, Q, from_square, to_square, true)));
							moves.push_back(encode_move(from_square, to_square, piece, R, 1, 0, 0, 0, get_piece_score(depth, P, R, from_square, to_square, true)));
							moves.push_back(encode_move(from_square, to_square, piece, B, 1, 0, 0, 0, get_piece_score(depth, P, B, from_square, to_square, true)));
							moves.push_back(encode_move(from_square, to_square, piece, N, 1, 0, 0, 0, get_piece_score(depth, P, N, from_square, to_square, true)));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					if (this->enpassant != no_sq)
					{
						uint64_t ep_attacks = pawn_attacks[white][from_square] & (1ULL << this->enpassant);

						if (ep_attacks)
						{
							uint8_t target_ep = bitwise::lsb(ep_attacks);
							moves.push_back(encode_move(from_square, target_ep, piece, 0, 1, 0, 1, 0, get_piece_score(depth, P, 0, from_square, target_ep, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, false)));
							}
						}
						else
						{				
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == K)
			{
				if (type == all_moves)
				{
					if (this->castling & white_oo)
					{
						if (!bitwise::check(this->occupied[both], f1) && !bitwise::check(this->occupied[both], g1))
						{
							if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
							{
								moves.push_back(encode_move(e1, g1, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e1, g1, false)));
							}
						}
					}

					if (this->castling & white_ooo)
					{
						if (!bitwise::check(this->occupied[both], d1) && !bitwise::check(this->occupied[both], c1) && !bitwise::check(this->occupied[both], b1))
						{
							if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
							{
								moves.push_back(encode_move(e1, c1, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e1, c1, false)));
							}
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, true)));
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
							moves.push_back(encode_move(from_square, to_square, piece, q, 0, 0, 0, 0, get_piece_score(depth, P, q, from_square, to_square, false)));
							moves.push_back(encode_move(from_square, to_square, piece, r, 0, 0, 0, 0, get_piece_score(depth, P, r, from_square, to_square, false)));
							moves.push_back(encode_move(from_square, to_square, piece, b, 0, 0, 0, 0, get_piece_score(depth, P, b, from_square, to_square, false)));
							moves.push_back(encode_move(from_square, to_square, piece, n, 0, 0, 0, 0, get_piece_score(depth, P, n, from_square, to_square, false)));
						}
						else
						{
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, false)));

								if ((from_square >= a7 && from_square <= h7) && !bitwise::check(this->occupied[both], to_square + 8))
								{
									moves.push_back(encode_move(from_square, to_square + 8, piece, 0, 0, 1, 0, 0, get_piece_score(depth, P, 0, from_square, to_square + 8, false)));
								}
							}
						}
					}
					
					attacks = pawn_attacks[black][from_square] & this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (from_square >= a2 && from_square <= h2)
						{
							moves.push_back(encode_move(from_square, to_square, piece, q, 1, 0, 0, 0, get_piece_score(depth, P, q, from_square, to_square, true)));
							moves.push_back(encode_move(from_square, to_square, piece, r, 1, 0, 0, 0, get_piece_score(depth, P, r, from_square, to_square, true)));
							moves.push_back(encode_move(from_square, to_square, piece, b, 1, 0, 0, 0, get_piece_score(depth, P, b, from_square, to_square, true)));
							moves.push_back(encode_move(from_square, to_square, piece, n, 1, 0, 0, 0, get_piece_score(depth, P, n, from_square, to_square, true)));
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					if (this->enpassant != no_sq)
					{
						uint64_t ep_attacks = pawn_attacks[black][from_square] & (1ULL << this->enpassant);

						if (ep_attacks)
						{
							uint8_t target_ep = bitwise::lsb(ep_attacks);
							moves.push_back(encode_move(from_square, target_ep, piece, 0, 1, 0, 1, 0, get_piece_score(depth, P, 0, from_square, target_ep, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, true)));
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
			else if (piece == k)
			{
				if (type == all_moves)
				{
					if (this->castling & black_oo)
					{
						if (!bitwise::check(this->occupied[both], f8) && !bitwise::check(this->occupied[both], g8))
						{
							if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
							{
								moves.push_back(encode_move(e8, g8, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e8, g8, false)));
							}
						}
					}

					if (this->castling & black_ooo)
					{
						if (!bitwise::check(this->occupied[both], d8) && !bitwise::check(this->occupied[both], c8) && !bitwise::check(this->occupied[both], b8))
						{
							if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
							{
								moves.push_back(encode_move(e8, c8, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e8, c8, false)));
							}
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
							if (type == all_moves)
							{
								moves.push_back(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push_back(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
		}
	}

	if (extract_legal) { moves = extract_legal_moves(moves); }

	if (sort)
	{
		if (sort) { std::sort(moves.begin(), moves.end(), [](const uint32_t& a, const uint32_t& b) { return ((a & 0x3f000000) >> 24) > ((b & 0x3f000000) >> 24); }); }
	}
}

std::vector<uint32_t> board::extract_legal_moves(std::vector<uint32_t> moves)
{
	std::vector<uint32_t> legal_moves;

	for (int i = 0; i < moves.size(); ++i)
	{
		uint8_t temp_side = this->side;
		board_undo undo_board;
		preserve_board(undo_board);
		make_move(moves[i], false);
		
		uint64_t kbb = this->state[side_to_piece_type[temp_side][K]];
		if (!is_square_attacked(bitwise::lsb(kbb), this->side))
		{
			legal_moves.push_back(moves[i]);
		}

		restore_board(undo_board);
	}

	return legal_moves;
}

void board::preserve_board(board_undo& undo_board)
{
	memcpy(undo_board.state, this->state, sizeof(this->state));
	memcpy(undo_board.occupied, this->occupied, sizeof(this->occupied));
	undo_board.side = this->side;
	undo_board.castling = this->castling;
	undo_board.enpassant = this->enpassant;
	undo_board.fifty_move = this->fifty_move;
	undo_board.hashkey = this->hashkey;
}

void board::restore_board(const board_undo& undo_board)
{
	memcpy(this->state, undo_board.state, sizeof(this->state));
	memcpy(this->occupied, undo_board.occupied, sizeof(this->occupied));
	this->side = undo_board.side;
	this->castling = undo_board.castling;
	this->enpassant = undo_board.enpassant;
	this->fifty_move = undo_board.fifty_move;
	this->hashkey = undo_board.hashkey;
}

bool board::push_history()
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
	return true;
}

bool board::pop_history()
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
	}
	return false;
}

uint32_t board::string_to_move(const std::string& move_str)
{
	uint8_t ff = helper::letter_to_file(move_str[0]);
	uint8_t fr = 8 - (move_str[1] - '0');
	uint8_t tf = helper::letter_to_file(move_str[2]);
	uint8_t tr = 8 - (move_str[3] - '0');

	uint8_t move_from = helper::rank_and_file_to_square(fr, ff);
	uint8_t move_to = helper::rank_and_file_to_square(tr, tf);
	uint8_t move_promoted = 0;

	if (move_str.size() > 4)
	{
		switch (move_str[4])
		{
			case 'n':
				if (this->side == white) { move_promoted = N; }
				else { move_promoted = n; }
				break;
			case 'b':
				if (this->side == white) { move_promoted = B; }
				else { move_promoted = b; }
				break;
			case 'r':
				if (this->side == white) { move_promoted = R; }
				else { move_promoted = r; }
				break;
			case 'q':
				if (this->side == white) { move_promoted = Q; }
				else { move_promoted = q; }
				break;
		}
	}

	std::vector<uint32_t> moves;
	generate_moves(moves, false, all_moves, false, 0);

	for (int i = 0; i < moves.size(); ++i)
	{
		if (get_move_from(moves[i]) == move_from && get_move_to(moves[i]) == move_to && get_move_promoted_piece(moves[i]) == move_promoted)
		{
			return moves[i];
		}
	}
	return 0;
}

std::string board::move_to_string(const uint32_t& move)
{
	std::string move_str;

	uint8_t from = get_move_from(move);
	uint8_t to = get_move_to(move);
	uint8_t promoted_piece = get_move_promoted_piece(move);

	move_str += square_to_coords[from];
	move_str += square_to_coords[to];

	if (promoted_piece) { move_str += tolower(pieces_to_ascii[promoted_piece]); }

	return move_str;
}

bool board::make_move(const uint32_t& move, const bool& save_to_history)
{
	fifty_move++;
	if (save_to_history) { push_history(); }

	uint8_t from = get_move_from(move);
	uint8_t to = get_move_to(move);
	uint8_t piece = get_move_piece(move);
	uint8_t promoted_piece = get_move_promoted_piece(move);
	uint8_t capture_flag = get_move_capture_flag(move);
	uint8_t double_push_flag = get_move_double_push_flag(move);
	uint8_t enpassant_flag = get_move_enpassant_flag(move);
	uint8_t castling_flag = get_move_castling_flag(move);

	if (piece == P || piece == p) { this->fifty_move = 0; }

	bitwise::clear(this->state[piece], from);
	bitwise::set(this->state[piece], to);

	this->hashkey ^= state_hashkey[piece][from];
	this->hashkey ^= state_hashkey[piece][to];

	if (capture_flag)
	{
		this->fifty_move = 0;
		uint8_t start_piece = side_to_piece_type[!this->side][P];
		uint8_t end_piece = side_to_piece_type[!this->side][K];

		for (uint8_t bb_piece = start_piece; bb_piece <= end_piece; ++bb_piece)
		{
			if (bitwise::check(this->state[bb_piece], to))
			{
				bitwise::clear(this->state[bb_piece], to);
				this->hashkey ^= state_hashkey[bb_piece][to];
				break;
			}
		}
	}

	if (promoted_piece)
	{
		bitwise::clear(this->state[side_to_piece_type[this->side][P]], to);
		bitwise::set(this->state[promoted_piece], to);

		this->hashkey ^= state_hashkey[side_to_piece_type[this->side][P]][to];
		this->hashkey ^= state_hashkey[promoted_piece][to];
	}

	if (enpassant_flag)
	{
		if (this->side == white)
		{
			bitwise::clear(this->state[p], to + 8);
			this->hashkey ^= state_hashkey[p][to + 8];
		}
		else
		{
			bitwise::clear(this->state[P], to - 8);
			this->hashkey ^= state_hashkey[P][to - 8];
		}								
	}

	if (this->enpassant != no_sq) { this->hashkey ^= enpassant_hashkey[this->enpassant]; }

	this->enpassant = no_sq;

	if (double_push_flag)
	{
		if (this->side == white)
		{
			this->enpassant = to + 8;
			this->hashkey ^= enpassant_hashkey[to + 8];
		}
		else
		{
			this->enpassant = to - 8;
			this->hashkey ^= enpassant_hashkey[to - 8];
		}
	}

	if (castling_flag)
	{
		this->fifty_move = 0;
		switch (to)
		{
			case g1:
				bitwise::clear(this->state[R], h1);
				bitwise::set(this->state[R], f1);
				this->hashkey ^= state_hashkey[R][h1];
				this->hashkey ^= state_hashkey[R][f1];
				break;
			case c1:
				bitwise::clear(this->state[R], a1);
				bitwise::set(this->state[R], d1);
				this->hashkey ^= state_hashkey[R][a1];
				this->hashkey ^= state_hashkey[R][d1];
				break;
			case g8:
				bitwise::clear(this->state[r], h8);
				bitwise::set(this->state[r], f8);
				this->hashkey ^= state_hashkey[r][h8];
				this->hashkey ^= state_hashkey[r][f8];
				break;
			case c8:
				bitwise::clear(this->state[r], a8);
				bitwise::set(this->state[r], d8);
				this->hashkey ^= state_hashkey[r][a8];
				this->hashkey ^= state_hashkey[r][d8];
				break;
		}
	}

	this->hashkey ^= castling_hashkey[this->castling];

	this->castling &= castling_rights[from];
	this->castling &= castling_rights[to];

	this->hashkey ^= castling_hashkey[this->castling];

	memset(this->occupied, 0ULL, sizeof(this->occupied));

	for (uint8_t i = P; i <= K; ++i) { this->occupied[white] |= this->state[i]; }

	for (uint8_t i = p; i <= k; ++i) { this->occupied[black] |= this->state[i]; }

	this->occupied[both] |= this->occupied[white];
	this->occupied[both] |= this->occupied[black];

	this->side ^= 1;
	this->hashkey ^= side_hashkey;

	return true;
}

int board::get_game_phase_score()
{
	int score = 0;

	for (uint8_t i = N; i <= Q; ++i) { score += (bitwise::count(this->state[i]) * material_evaluation[opening][i]); }

	for (uint8_t i = n; i <= q; ++i) { score += (bitwise::count(this->state[i]) * -material_evaluation[opening][i]); }

	return score;
}

int board::evaluate()
{
	int score = 0;
	int doubled_pawns = 0;
	int game_phase_score = get_game_phase_score();
	uint64_t bb = 0ULL;

	int center_control = bitwise::count(this->occupied[white] & extended_center) - bitwise::count(this->occupied[black] & extended_center);
	score += (center_control * 2);

	int closed_position_count = bitwise::count((this->state[P] | this->state[p]));

	int white_color_complex = bitwise::count(this->state[P] & light_squares) - bitwise::count(this->state[P] & dark_squares);
	int black_color_complex = bitwise::count(this->state[p] & light_squares) - bitwise::count(this->state[p] & dark_squares);
	
	if (white_color_complex > 0)
	{
		if (!(this->state[B] & dark_squares)) 
		{ 
			score -= (abs(white_color_complex) * 3);
			if ((this->state[b] & dark_squares)) { score -= (abs(white_color_complex) * 4); }
		}
	}
	else if (white_color_complex < 0)
	{
		if (!(this->state[B] & light_squares)) 
		{ 
			score -= (abs(white_color_complex) * 3); 
			if ((this->state[b] & light_squares)) { score -= (abs(white_color_complex) * 4); }
		}
	}

	if (black_color_complex > 0)
	{
		if (!(this->state[b] & dark_squares))
		{ 
			score += (abs(black_color_complex) * 3);
			if ((this->state[B] & dark_squares)) { score += (abs(black_color_complex) * 4); }
		}
	}
	else if (black_color_complex < 0)
	{
		if (!(this->state[b] & light_squares))
		{ 
			score += (abs(black_color_complex) * 3);
			if ((this->state[B] & light_squares)) { score += (abs(black_color_complex) * 4); }
		}
	}

	for (uint8_t piece = P; piece <= k; ++piece)
	{
		uint64_t bitboard = this->state[piece];

		while (bitboard)
		{
			uint8_t square = bitwise::lsb(bitboard);

			score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, material_evaluation[endgame][piece], material_evaluation[opening][piece]);

			switch (piece)
			{
			case P:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][P][square], positional_evaluation[opening][P][square]);				
				doubled_pawns = bitwise::count(this->state[P] & file_masks_by_square[square]);
				if (doubled_pawns > 1) { score -= ((doubled_pawns - 1) * 15); }
				bb = (this->state[P] & isolated_masks_by_square[square]);
				if (!bb) { score -= 10; }
				bb = (this->state[p] & white_passed_masks_by_square[square]);
				if (!bb) { score += passed_pawn_evaluation[rank_by_square[square]]; }
				break;
			case N:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][N][square], positional_evaluation[opening][N][square]);
				score += bitwise::count(knight_attacks[square] & ~this->occupied[white]);
				if (closed_position_count > 12) { score += 10; }
				break;
			case B:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][B][square], positional_evaluation[opening][B][square]);
				score += ((bitwise::count(bishop_attacks(square) & ~this->occupied[both]) - 4) * 5);
				score += 15;
				break;
			case R:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][R][square], positional_evaluation[opening][R][square]);
				bb = (this->state[P] & file_masks_by_square[square]);
				if (!bb) { score += 10; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score += 15; }
				break;
			case Q:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][Q][square], positional_evaluation[opening][Q][square]);
				break;
			case K:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][K][square], positional_evaluation[opening][K][square]);
				bb = (this->state[P] & file_masks_by_square[square]);
				if (!bb) { score -= 20; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score -= 30; }		
				break;
			case p:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][P][mirror_square[square]], positional_evaluation[opening][P][mirror_square[square]]);
				doubled_pawns = bitwise::count(this->state[p] & file_masks_by_square[square]);
				if (doubled_pawns > 1) { score += ((doubled_pawns - 1) * 15); }
				bb = (this->state[p] & isolated_masks_by_square[square]);
				if (!bb) { score += 10; }
				bb = (this->state[P] & black_passed_masks_by_square[square]);
				if (!bb) { score -= passed_pawn_evaluation[rank_by_square[square]]; }
				break;
			case n:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][N][mirror_square[square]], positional_evaluation[opening][N][mirror_square[square]]);
				score -= bitwise::count(knight_attacks[square] & ~this->occupied[black]);
				if (closed_position_count > 12) { score -= 10; }
				break;
			case b:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][B][mirror_square[square]], positional_evaluation[opening][B][mirror_square[square]]);
				score -= ((bitwise::count(bishop_attacks(square) & ~this->occupied[both]) - 4) * 5);
				score -= 15;
				break;
			case r:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][R][mirror_square[square]], positional_evaluation[opening][R][mirror_square[square]]);
				bb = (this->state[p] & file_masks_by_square[square]);
				if (!bb) { score -= 10; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score -= 15; }
				break;
			case q:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][Q][mirror_square[square]], positional_evaluation[opening][Q][mirror_square[square]]);
				break;
			case k:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][K][mirror_square[square]], positional_evaluation[opening][K][mirror_square[square]]);
				bb = (this->state[p] & file_masks_by_square[square]);
				if (!bb) { score += 20; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score += 30; }
				break;
			}
			bitwise::clear(bitboard, square);
		}
	}

	return (!this->side ? score : -score);
}