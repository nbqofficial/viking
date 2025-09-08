#include "uci.h"

uci::uci()
{
	console_handle = GetStdHandle(STD_INPUT_HANDLE);
}

uci::~uci()
{
	CloseHandle(console_handle);
}

void uci::go(char* line_in)
{
	int depth = -1;
	int movestogo = 30;
	int movetime = -1;
	int time = -1;
	int inc = 0;
	char* ptr = NULL;
	uci_info.timeset = false;

	if ((ptr = strstr(line_in, "infinite")))
	{
		; // do nothing
	}

	if ((ptr = strstr(line_in, "binc")) && this->b.get_side() == black)
	{
		inc = atoi(ptr + 5);
	}
	if ((ptr = strstr(line_in, "winc")) && this->b.get_side() == white)
	{
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line_in, "btime")) && this->b.get_side() == black)
	{
		time = atoi(ptr + 6);
	}
	if ((ptr = strstr(line_in, "wtime")) && this->b.get_side() == white)
	{
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line_in, "movestogo")))
	{
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line_in, "movetime")))
	{
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line_in, "depth")))
	{
		depth = atoi(ptr + 6);
	}

	if (movetime != -1)
	{
		time = movetime;
		movestogo = 1;
	}

	uci_info.start_time = helper::get_time_ms();
	uci_info.depth = depth;

	if (time != -1)
	{
		uci_info.timeset = true;
		time /= movestogo;
		time -= 50;
		uci_info.stop_time = uci_info.start_time + time + inc;
	}

	if (depth == -1)
	{
		uci_info.depth = MAX_DEPTH;
	}

	uint32_t best_move = this->sc.go(this->b, uci_info.depth, true, this->debug);
	std::string move_str = this->b.move_to_string(best_move);
	this->b.make_move(best_move, true);

	if (this->display_after_move)
	{
		system("cls");
		this->b.display();
	}

	printf("bestmove %s\n", move_str.c_str());
}

void uci::position(char* line_in)
{
	line_in += 9;
	char* ptr = line_in;

	if (strncmp(line_in, "startpos", 8) == 0)
	{
		this->b.init(start_position, false);
	}
	else
	{
		ptr = strstr(line_in, "fen");
		if (ptr == NULL)
		{
			b.init(start_position, false);
		}
		else
		{
			ptr += 4;
			this->b.init(ptr, false);
		}
	}

	ptr = strstr(line_in, "moves");

	if (ptr != NULL)
	{
		ptr += 6;
		while (*ptr)
		{
			std::string move;
			while (*ptr && *ptr != ' ')
			{
				move += ptr;
				ptr++;
			}
			this->b.make_move(this->b.string_to_move(move), true);
			move.clear();
			ptr++;
		}
	}
}

void uci::parse_perft(char* line_in)
{
	line_in += 6;
	char* ptr = line_in;
	if (ptr == NULL)
	{
		this->pf.test(this->b, 5);
	}
	else
	{
		this->pf.test(this->b, atoi(ptr));
	}
}

void uci::parse_displaymoves()
{
	move_list moves{};
	b.generate_moves(moves, true, all_moves, true, 0);

	this->b.display_moves(moves);
}

void uci::parse_displayboard()
{
	this->b.display();
}

void uci::parse_debug()
{
	this->debug = !this->debug;
	if (this->debug)
	{
		printf("\tdebug on\n");
	}
	else
	{
		printf("\tdebug off\n");
	}
}

void uci::parse_displayaftermove()
{
	this->display_after_move = !this->display_after_move;
	if (this->display_after_move)
	{
		printf("\t display after move on\n");
	}
	else
	{
		printf("\t display after move off\n");
	}
}

void uci::uci_loop()
{
	setvbuf(stdin, NULL, _IONBF, BUFSIZ);
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	char line[MAX_INPUT_BUFFER];

	printf("id name %s\n", ENGINE_NAME);
	printf("id author %s\n", ENGINE_AUTHOR);
	printf("uciok\n");

	while (1)
	{
		memset(&line[0], 0, sizeof(line));
		fflush(stdout);

		if (!fgets(line, MAX_INPUT_BUFFER, stdin)) { continue; }

		if (line[0] == '\n') { continue; }

		if (!strncmp(line, "isready", 7))
		{
			printf("readyok\n");
			continue;
		}
		else if (!strncmp(line, "position", 8))
		{
			position(line);
		}
		else if (!strncmp(line, "ucinewgame", 10))
		{
			position((char*)"position startpos\n");
		}
		else if (!strncmp(line, "go", 2))
		{
			go(line);
		}
		else if (!strncmp(line, "quit", 4))
		{
			exit(EXIT_SUCCESS);
			break;
		}
		else if (!strncmp(line, "uci", 3))
		{
			printf("id name %s\n", ENGINE_NAME);
			printf("id author %s\n", ENGINE_AUTHOR);
			printf("uciok\n");
		}
		else if (!strncmp(line, "perft", 5))
		{
			parse_perft(line);
		}
		else if (!strncmp(line, "displayboard", 12))
		{
			parse_displayboard();
		}
		else if (!strncmp(line, "displaymoves", 12))
		{
			parse_displaymoves();
		}
		else if (!strncmp(line, "displayaftermove", 16))
		{
			parse_displayaftermove();
		}
		else if (!strncmp(line, "cls", 3))
		{
			system("cls");
		}
		else if (!strncmp(line, "debug", 5))
		{
			parse_debug();
		}
	}
}
