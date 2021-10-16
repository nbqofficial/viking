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

void board::encode_move(uint32_t& move, const uint8_t& from, const uint8_t& to, const uint8_t& piece, const uint8_t& promoted_piece, const uint8_t& capture_flag, const uint8_t& double_push_flag, const uint8_t& enpassant_flag, const uint8_t& castling_flag, const uint8_t& mvvlva)
{
	move |= (from);
	move |= (to << 6);
	move |= (piece << 12);
	move |= (promoted_piece << 16);
	move |= (capture_flag << 20);
	move |= (double_push_flag << 21);
	move |= (enpassant_flag << 22);
	move |= (castling_flag << 23);
	move |= (mvvlva << 24);
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

void board::display_moves(const std::vector<uint32_t>& move_list)
{
	printf("\tmove    piece promoted capture doublepush enpassant castling mvvlva\n\n");
	for (int i = 0; i < move_list.size(); ++i)
	{
		display_move(move_list[i]);
	}
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

