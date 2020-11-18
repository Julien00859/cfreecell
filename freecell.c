#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define SPADE 0
#define CLUB 16
#define HEART 32
#define DIAMOND 48

typedef Card uint8_t;

typedef struct {
	Card fc_slots[4];       // freecells

	Card fd_slots[4][14];   // foundation
	int fd_slots_top[4];    // index of card on top of each foundation slot

	Card columns[8][20];    // columns
	int columns_bot[8];     // index of card on bottom of each column
} Board;


void coltocol(Board *board, int fromcol, int tocol) {

	int fromrow, torow;
	Card fromcard, tocard;

	fromrow = board->columns_bot[fromcol];
	torow = board->columns_bot[tocol];

	if (fromrow == -1) return;
	fromcard = board->columns[fromcol][fromrow];

	if (torow >= 0) {
		tocard = board->columns[tocol][torow];
		if (fromcard & colormask == tocard && colormask) return;
		if (fromcard & valuemask != tocard && valuemask - 1) return;
	}
}



uint8_t placeholder = 0b01000000;
uint8_t blankcard   = 0b01000001;
uint8_t colormask   = 0b00100000;  // 0 black, 1 red
uint8_t symbolmask  = 0b00110000;  // 0 spade, 1 club, 2 heart, 3 diamond
uint8_t valuemask   = 0b00001111;  // 1 ace, 2 two, ..., 10 ten, ..., 13 king

void board_clear(Board *board) {
	int row, col;

	for (col = 0; col < 4; col++) {
		board->fc_slots[col] = placeholder;
	}

	for (col = 0; col < 4; col++) {
		board->fd_slots[col][0] = placeholder;
		board->fd_slots_top[col] = &(board->fd_slots[col]);
		for (row = 1; row < 14; row++) {
			board->fd_slots[col][row] = blankcard;
		}
	}

	for (col = 0; col < 8; col++) {
		board->columns_bot[col] = NULL;
		for (row = 0; row < 20; row++) {
			board->columns[col][row] = blankcard;
		}
	}
}

void board_init(Board *board) {
	uint8_t fulldeck[52];
	uint8_t value, symbol;
	int row, col;

	for (symbol = 0; symbol < 4; symbol++) {
		for (value = 0; value < 14; value++) {
			fulldeck[symbol * 13 + value] = symbol * 16 + value + 1;
		}
	}

	//shuffle(fulldeck);

	for (row = 0; row < 6; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[row][col] = fulldeck[row * 8 + col];
		}
	}
	for (col = 0; col < 4; col++) {
		board->columns[6][col] = fulldeck[48 + col];
		board->columns_bot[6] = &(board->columns[6][col]);
	}
	for (; col < 8; col++) {
		board->columns_bot[5] = &(board->columns[5][col]);
	}
}

void card_str(uint8_t card, char *str) {
	if (card == blankcard) {
		str[0] = str[1] = str[2] = ' ';
		return;
	}
	if (card == placeholder) {
		str[0] = str[2] = ' ';
		str[1] = '_';
		return;
	}
	str[0] = ' ';
	switch (card & valuemask) {
		case 10: str[0] = '1'; str[1] = '0'; break;
		case 11: str[1] = 'J'; break;
		case 12: str[1] = 'Q'; break;
		case 13: str[1] = 'K'; break;
		default: str[1] = '0' + (card & valuemask);
	}
	switch (card & symbolmask) {
		case SPADE: str[2] = 'S'; break;
		case CLUB: str[2] = 'C'; break;
		case HEART: str[2] = 'h'; break;
		case DIAMOND: str[2] = 'd'; break;
	}
}

void board_show(Board *board) {
	int row, col;
	char str[4];
	str[3] = 0;

	for (col = 0; col < 4; col++) {
		card_str(board->fc_slots[col], str);
		printf("%s ", str);
	}
	printf("|");
	for (col = 0; col < 4; col++) {
		card_str(board->fd_slots[col][board->fd_slots_i[col]], str);
		printf(" %s", str);
	}
	printf("\n---------------------------------\n");
	for (row = 0; row < 20; row++) {
		for (col = 0; col < 8; col++) {
			card_str(board->columns[row][col], str);
			printf(" %s", str);
		}
		printf("\n");
	}
}



int main(void) {
	Board board;
	board_clear(&board);
	board_init(&board);
	board_show(&board);
	return 0;
}
