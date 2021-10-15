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

bool board::init(const std::string& fen, const bool& display_fen)
{
	if (display_fen) { printf("FEN: %s\n", fen.c_str()); }

	this->reset();

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
			if (!f) { printf("  %d  ", 8 - r); }
			
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
	printf("\n     a b c d e f g h\n\n");

	printf("     Side: %s\n", !this->side ? "white" : "black");

	printf("     Castling: %c%c%c%c\n", (this->castling & white_oo) ? 'K' : '-',
								   (this->castling & white_ooo) ? 'Q' : '-', 
								   (this->castling & black_oo) ? 'k' : '-', 
								   (this->castling & black_ooo) ? 'q' : '-');

	printf("     Enpassant: %s\n", (this->enpassant != no_sq) ? square_to_coords[this->enpassant] : "-");

	printf("     Fifty Moves: %d\n", this->fifty_move);
	printf("     Ply: %d\n", this->ply);
	printf("     Hashkey: %llu\n", this->hashkey);
	printf("     History Size: %llu\n", this->history.size());
}
