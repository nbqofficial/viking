#include "board.h"

board::board() noexcept
{

}

board::~board() noexcept
{
}

bool board::init(const std::string& fen, bool display_fen) noexcept
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

void board::display() noexcept
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
	printf("\tevaluation:\t%d\n", this->evaluate());
	printf("\t %llx\n\n", this->hashkey);
}

void board::display_move(uint32_t move) const noexcept
{
	uint8_t from = n_move::get_move_from(move);
	uint8_t to = n_move::get_move_to(move);
	uint8_t piece = n_move::get_move_piece(move);
	uint8_t promoted_piece = n_move::get_move_promoted_piece(move);
	uint8_t capture_flag = n_move::get_move_capture_flag(move);
	uint8_t double_push_flag = n_move::get_move_double_push_flag(move);
	uint8_t enpassant_flag = n_move::get_move_enpassant_flag(move);
	uint8_t castling_flag = n_move::get_move_castling_flag(move);
	uint8_t mvvlva = n_move::get_move_score(move);

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

void board::display_moves(const move_list& moves) const noexcept
{
	printf("\tmove    piece promoted capture doublepush enpassant castling score\n\n");
	for (int i = 0; i < moves.m_size; ++i)
	{
		display_move(moves.m_moves[i]);
	}
	printf("\n\ttotal moves: %d\n", moves.m_size);
}

void board::display_pv_debug(const std::vector<uint32_t>& pv, int depth) const noexcept
{
	printf("\t%d          ", depth);
	for (int i = 0; i < pv.size(); ++i)
	{
		uint8_t from = n_move::get_move_from(pv[i]);
		uint8_t to = n_move::get_move_to(pv[i]);
		uint8_t promoted_piece = n_move::get_move_promoted_piece(pv[i]);

		printf("%s%s%c ", square_to_coords[from],
						  square_to_coords[to],
						  promoted_piece ? tolower(pieces_to_ascii[promoted_piece]) : ' ');
	}
	printf("\n");
}

void board::display_info(const std::vector<uint32_t>& pv, int score, int depth, long long nodes) const noexcept
{
	printf("info score cp %d depth %d nodes %lld pv ", score, depth, nodes);

	for (int i = 0; i < pv.size(); ++i)
	{
		uint8_t from = n_move::get_move_from(pv[i]);
		uint8_t to = n_move::get_move_to(pv[i]);
		uint8_t promoted_piece = n_move::get_move_promoted_piece(pv[i]);

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

void board::generate_hashkey() noexcept
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

uint8_t board::get_piece_score(int depth, uint8_t piece, uint8_t promoted_piece, uint8_t from_square, uint8_t to_square, bool is_capture) noexcept
{
	if (this->pv_line.size() && n_move::get_move_from(this->pv_line[depth]) == from_square && n_move::get_move_to(this->pv_line[depth]) == to_square && n_move::get_move_promoted_piece(this->pv_line[depth]) == promoted_piece)
	{
		return 200;
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
		if (n_move::get_move_from(this->killer_moves[0][depth]) == from_square && n_move::get_move_to(this->killer_moves[0][depth]) == to_square && n_move::get_move_promoted_piece(this->killer_moves[0][depth]) == promoted_piece)
		{
			return 90;
		}
		else if (n_move::get_move_from(this->killer_moves[1][depth]) == from_square && n_move::get_move_to(this->killer_moves[1][depth]) == to_square && n_move::get_move_promoted_piece(this->killer_moves[1][depth]) == promoted_piece)
		{
			return 80;
		}	
		else
		{
			uint8_t hms = this->history_moves[side_to_piece_type[this->side][piece]][to_square];
			if (hms > 70) { return hms; }
			else { return this->score_possible_attack(piece, to_square); }
		}
	}
	return 0;
}

void board::generate_pseudolegal(move_list& moves, uint8_t type, int depth) noexcept
{
	moves.clear();

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
							moves.push(n_move::encode_move(from_square, to_square, piece, Q, 0, 0, 0, 0, get_piece_score(depth, P, Q, from_square, to_square, false)));
							moves.push(n_move::encode_move(from_square, to_square, piece, R, 0, 0, 0, 0, get_piece_score(depth, P, R, from_square, to_square, false)));
							moves.push(n_move::encode_move(from_square, to_square, piece, B, 0, 0, 0, 0, get_piece_score(depth, P, B, from_square, to_square, false)));
							moves.push(n_move::encode_move(from_square, to_square, piece, N, 0, 0, 0, 0, get_piece_score(depth, P, N, from_square, to_square, false)));
						}
						else
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, false)));

								if ((from_square >= a2 && from_square <= h2) && !bitwise::check(this->occupied[both], to_square - 8))
								{
									moves.push(n_move::encode_move(from_square, to_square - 8, piece, 0, 0, 1, 0, 0, get_piece_score(depth, P, 0, from_square, to_square - 8, false)));
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
							moves.push(n_move::encode_move(from_square, to_square, piece, Q, 1, 0, 0, 0, get_piece_score(depth, P, Q, from_square, to_square, true)));
							moves.push(n_move::encode_move(from_square, to_square, piece, R, 1, 0, 0, 0, get_piece_score(depth, P, R, from_square, to_square, true)));
							moves.push(n_move::encode_move(from_square, to_square, piece, B, 1, 0, 0, 0, get_piece_score(depth, P, B, from_square, to_square, true)));
							moves.push(n_move::encode_move(from_square, to_square, piece, N, 1, 0, 0, 0, get_piece_score(depth, P, N, from_square, to_square, true)));
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					if (this->enpassant != no_sq)
					{
						uint64_t ep_attacks = pawn_attacks[white][from_square] & (1ULL << this->enpassant);

						if (ep_attacks)
						{
							uint8_t target_ep = bitwise::lsb(ep_attacks);
							moves.push(n_move::encode_move(from_square, target_ep, piece, 0, 1, 0, 1, 0, get_piece_score(depth, P, 0, from_square, target_ep, true)));
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
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, true)));
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

					attacks = this->bishop_attacks(from_square) & ~this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, true)));
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

					attacks = this->rook_attacks(from_square) & ~this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, true)));
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

					attacks = this->rook_attacks(from_square) | this->bishop_attacks(from_square);
					attacks &= ~this->occupied[white];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[black], to_square))
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, true)));
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
							if (!this->is_square_attacked(e1, black) && !this->is_square_attacked(f1, black))
							{
								moves.push(n_move::encode_move(e1, g1, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e1, g1, false)));
							}
						}
					}

					if (this->castling & white_ooo)
					{
						if (!bitwise::check(this->occupied[both], d1) && !bitwise::check(this->occupied[both], c1) && !bitwise::check(this->occupied[both], b1))
						{
							if (!this->is_square_attacked(e1, black) && !this->is_square_attacked(d1, black))
							{
								moves.push(n_move::encode_move(e1, c1, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e1, c1, false)));
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
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, true)));
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
							moves.push(n_move::encode_move(from_square, to_square, piece, q, 0, 0, 0, 0, get_piece_score(depth, P, q, from_square, to_square, false)));
							moves.push(n_move::encode_move(from_square, to_square, piece, r, 0, 0, 0, 0, get_piece_score(depth, P, r, from_square, to_square, false)));
							moves.push(n_move::encode_move(from_square, to_square, piece, b, 0, 0, 0, 0, get_piece_score(depth, P, b, from_square, to_square, false)));
							moves.push(n_move::encode_move(from_square, to_square, piece, n, 0, 0, 0, 0, get_piece_score(depth, P, n, from_square, to_square, false)));
						}
						else
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, false)));

								if ((from_square >= a7 && from_square <= h7) && !bitwise::check(this->occupied[both], to_square + 8))
								{
									moves.push(n_move::encode_move(from_square, to_square + 8, piece, 0, 0, 1, 0, 0, get_piece_score(depth, P, 0, from_square, to_square + 8, false)));
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
							moves.push(n_move::encode_move(from_square, to_square, piece, q, 1, 0, 0, 0, get_piece_score(depth, P, q, from_square, to_square, true)));
							moves.push(n_move::encode_move(from_square, to_square, piece, r, 1, 0, 0, 0, get_piece_score(depth, P, r, from_square, to_square, true)));
							moves.push(n_move::encode_move(from_square, to_square, piece, b, 1, 0, 0, 0, get_piece_score(depth, P, b, from_square, to_square, true)));
							moves.push(n_move::encode_move(from_square, to_square, piece, n, 1, 0, 0, 0, get_piece_score(depth, P, n, from_square, to_square, true)));
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, P, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					if (this->enpassant != no_sq)
					{
						uint64_t ep_attacks = pawn_attacks[black][from_square] & (1ULL << this->enpassant);

						if (ep_attacks)
						{
							uint8_t target_ep = bitwise::lsb(ep_attacks);
							moves.push(n_move::encode_move(from_square, target_ep, piece, 0, 1, 0, 1, 0, get_piece_score(depth, P, 0, from_square, target_ep, true)));
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
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, N, 0, from_square, to_square, true)));
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

					attacks = this->bishop_attacks(from_square) & ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, B, 0, from_square, to_square, true)));
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

					attacks = this->rook_attacks(from_square) & ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, R, 0, from_square, to_square, true)));
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

					attacks = this->rook_attacks(from_square) | this->bishop_attacks(from_square);
					attacks &= ~this->occupied[black];

					while (attacks)
					{
						to_square = bitwise::lsb(attacks);

						if (!bitwise::check(this->occupied[white], to_square))
						{
							if (type == all_moves)
							{
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, Q, 0, from_square, to_square, true)));
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
							if (!this->is_square_attacked(e8, white) && !this->is_square_attacked(f8, white))
							{
								moves.push(n_move::encode_move(e8, g8, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e8, g8, false)));
							}
						}
					}

					if (this->castling & black_ooo)
					{
						if (!bitwise::check(this->occupied[both], d8) && !bitwise::check(this->occupied[both], c8) && !bitwise::check(this->occupied[both], b8))
						{
							if (!this->is_square_attacked(e8, white) && !this->is_square_attacked(d8, white))
							{
								moves.push(n_move::encode_move(e8, c8, piece, 0, 0, 0, 0, 1, get_piece_score(depth, K, 0, e8, c8, false)));
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
								moves.push(n_move::encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, false)));
							}
						}
						else
						{
							moves.push(n_move::encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0, get_piece_score(depth, K, 0, from_square, to_square, true)));
						}
						bitwise::clear(attacks, to_square);
					}

					bitwise::clear(bitboard, from_square);
				}
			}
		}
	}
}

void board::generate_moves(move_list& moves, bool sort, uint8_t type, bool extract_legal, int depth) noexcept
{
	if (extract_legal)
	{
		move_list pseudo{};
		this->generate_pseudolegal(pseudo, type, depth);
		moves.clear();
		const uint8_t us = this->side;

		for (uint8_t i = 0; i < pseudo.m_size; ++i)
		{
			const uint32_t move = pseudo.m_moves[i];
			board_undo undo_board;
			preserve_board(undo_board);
			make_move(move, false);

			const uint64_t kbb = this->state[side_to_piece_type[us][K]];
			const bool safe = !this->is_square_attacked(bitwise::lsb(kbb), this->side);
			if (safe) { moves.push(move); }

			restore_board(undo_board);
		}
	}
	else
	{
		moves.clear();
		this->generate_pseudolegal(moves, type, depth);
	}

	if (sort)
	{
		n_move::sort_moves_by_score(moves);
	}
}

bool board::make_move(uint32_t move, bool save_to_history) noexcept
{
	fifty_move++;
	if (save_to_history) { push_history(); }

	uint8_t from = n_move::get_move_from(move);
	uint8_t to = n_move::get_move_to(move);
	uint8_t piece = n_move::get_move_piece(move);
	uint8_t promoted_piece = n_move::get_move_promoted_piece(move);
	uint8_t capture_flag = n_move::get_move_capture_flag(move);
	uint8_t double_push_flag = n_move::get_move_double_push_flag(move);
	uint8_t enpassant_flag = n_move::get_move_enpassant_flag(move);
	uint8_t castling_flag = n_move::get_move_castling_flag(move);

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

int board::evaluate() noexcept
{
	int score = 0;
	int doubled_pawns = 0;
	int game_phase_score = get_game_phase_score();
	uint64_t bb = 0ULL;

	int space_control = (bitwise::count(this->occupied[white] & black_territory) - bitwise::count(this->occupied[black] & white_territory)) * 5;
	score += space_control;

	int white_kingside_pawns = bitwise::count(this->state[P] & king_side) * 3;
	int white_queenside_pawns = bitwise::count(this->state[P] & queen_side) * 2;

	int black_kingside_pawns = bitwise::count(this->state[p] & king_side) * 3;
	int black_queenside_pawns = bitwise::count(this->state[p] & queen_side) * 2;

	int pawn_side_score = white_kingside_pawns + white_queenside_pawns - black_kingside_pawns - black_queenside_pawns;
	score += pawn_side_score;

	int center_control = (bitwise::count(this->occupied[white] & extended_center) - bitwise::count(this->occupied[black] & extended_center)) * 15;
	score += center_control;

	int closed_position_count = bitwise::count((this->state[P] | this->state[p]));

	int white_color_complex = bitwise::count(this->state[P] & light_squares) - bitwise::count(this->state[P] & dark_squares);
	int black_color_complex = bitwise::count(this->state[p] & light_squares) - bitwise::count(this->state[p] & dark_squares);
	
	// tempo bonus
	if (this->side == white) { score += 15; }
	else { score -= 15; }

	if (white_color_complex > 0)
	{
		if (!(this->state[B] & dark_squares)) 
		{ 
			score -= (abs(white_color_complex) * 3);
			if ((this->state[b] & dark_squares)) { score -= (abs(white_color_complex) * 2); }
		}
	}
	else if (white_color_complex < 0)
	{
		if (!(this->state[B] & light_squares)) 
		{ 
			score -= (abs(white_color_complex) * 3); 
			if ((this->state[b] & light_squares)) { score -= (abs(white_color_complex) * 2); }
		}
	}

	if (black_color_complex > 0)
	{
		if (!(this->state[b] & dark_squares))
		{ 
			score += (abs(black_color_complex) * 3);
			if ((this->state[B] & dark_squares)) { score += (abs(black_color_complex) * 2); }
		}
	}
	else if (black_color_complex < 0)
	{
		if (!(this->state[b] & light_squares))
		{ 
			score += (abs(black_color_complex) * 3);
			if ((this->state[B] & light_squares)) { score += (abs(black_color_complex) * 2); }
		}
	}

	uint64_t white_king_ring2 = king_ring2[bitwise::lsb(this->state[K])];
	uint64_t black_king_ring2 = king_ring2[bitwise::lsb(this->state[k])];

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
				if (doubled_pawns > 1) { score -= ((doubled_pawns - 1) * 3); }
				bb = (this->state[P] & isolated_masks_by_square[square]);
				if (!bb) { score -= 10; }
				bb = (this->state[p] & white_passed_masks_by_square[square]);
				if (!bb) { score += passed_pawn_evaluation[rank_by_square[square]]; }
				if (bitwise::check(black_king_ring2, square)) { score += 2; }
				if (!this->is_defended_by_pawns(square, white)) { score -= 2; }
				break;
			case N:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][N][square], positional_evaluation[opening][N][square]);
				score += bitwise::count(knight_attacks[square] & ~this->occupied[white]);
				if (closed_position_count > 13) { score += 30; }
				if (bitwise::check(black_king_ring2, square)) { score += 10; }
				if (!this->is_defended_by_pawns(square, white)) { score -= 5; }
				break;
			case B:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][B][square], positional_evaluation[opening][B][square]);
				score += ((bitwise::count(bishop_attacks(square) & ~this->occupied[white]) - 4) * 5);
				score += 15;
				if (bitwise::check(black_king_ring2, square)) { score += 2; }
				break;
			case R:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][R][square], positional_evaluation[opening][R][square]);
				bb = (this->state[P] & file_masks_by_square[square]);
				if (!bb) { score += 5; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score += 15; }
				if (bitwise::check(black_king_ring2, square)) { score += 3; }
				score += ((bitwise::count(rook_attacks(square) & ~this->occupied[white]) - 7) * 2);
				break;
			case Q:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][Q][square], positional_evaluation[opening][Q][square]);
				if (bitwise::check(black_king_ring2, square)) { score += 3; }
				score += ((bitwise::count((bishop_attacks(square) | rook_attacks(square)) & ~this->occupied[white]) - 14) * 1);
				break;
			case K:
				score += helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][K][square], positional_evaluation[opening][K][square]);
				bb = (this->state[P] & file_masks_by_square[square]);
				if (!bb) { score -= 30; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score -= 60; }		
				break;
			case p:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][P][mirror_square[square]], positional_evaluation[opening][P][mirror_square[square]]);
				doubled_pawns = bitwise::count(this->state[p] & file_masks_by_square[square]);
				if (doubled_pawns > 1) { score += ((doubled_pawns - 1) * 3); }
				bb = (this->state[p] & isolated_masks_by_square[square]);
				if (!bb) { score += 10; }
				bb = (this->state[P] & black_passed_masks_by_square[square]);
				if (!bb) { score -= passed_pawn_evaluation[7 - rank_by_square[square]]; }
				if (bitwise::check(white_king_ring2, square)) { score -= 2; }
				if (!this->is_defended_by_pawns(square, black)) { score += 2; }
				break;
			case n:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][N][mirror_square[square]], positional_evaluation[opening][N][mirror_square[square]]);
				score -= bitwise::count(knight_attacks[square] & ~this->occupied[black]);
				if (closed_position_count > 13) { score -= 30; }
				if (bitwise::check(white_king_ring2, square)) { score -= 10; }
				if (!this->is_defended_by_pawns(square, black)) { score += 5; }
				break;
			case b:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][B][mirror_square[square]], positional_evaluation[opening][B][mirror_square[square]]);
				score -= ((bitwise::count(bishop_attacks(square) & ~this->occupied[black]) - 4) * 5);
				score -= 15;
				if (bitwise::check(white_king_ring2, square)) { score -= 2; }
				break;
			case r:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][R][mirror_square[square]], positional_evaluation[opening][R][mirror_square[square]]);
				bb = (this->state[p] & file_masks_by_square[square]);
				if (!bb) { score -= 5; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score -= 15; }
				if (bitwise::check(white_king_ring2, square)) { score -= 3; }
				score -= ((bitwise::count(rook_attacks(square) & ~this->occupied[black]) - 7) * 2);
				break;
			case q:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][Q][mirror_square[square]], positional_evaluation[opening][Q][mirror_square[square]]);
				if (bitwise::check(white_king_ring2, square)) { score -= 3; }
				score -= ((bitwise::count((bishop_attacks(square) | rook_attacks(square)) & ~this->occupied[black]) - 14) * 1);
				break;
			case k:
				score -= helper::taper(game_phase_score, GAME_PHASE_LOWBOUND, GAME_PHASE_HIGHBOUND, positional_evaluation[endgame][K][mirror_square[square]], positional_evaluation[opening][K][mirror_square[square]]);
				bb = (this->state[p] & file_masks_by_square[square]);
				if (!bb) { score += 30; }
				bb = ((this->state[P] | this->state[p]) & file_masks_by_square[square]);
				if (!bb) { score += 60; }
				break;
			}
			bitwise::clear(bitboard, square);
		}
	}

	return (!this->side ? score : -score);
}
