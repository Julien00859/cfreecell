#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct card {
	unsigned int color:1;
	unsigned int symbol:1;
	unsigned int value:4;
	unsigned int _fill:2;
} Card;


typedef struct board {
	Card freecell[4];

	Card foundation[4][14];
	uint8_t fdlen[4];

	Card columns[8][21];
	uint8_t collen[8];
} Board;


bool validate_move(Card cardfrom, Card cardto) {
	if (cardfrom.value == 0)
		return false;
	if (cardto.value == 0)
		return true;
	if (cardfrom.color == cardto.color)
		return false;
	if (cardfrom.value != cardto.value + 1)
		return false;
	return true;
}


void board_init(Board *board) {
	int row, col;
	int symbol, color, value;
	Card deck[52];
	Card newcard;

	memset(board, 0, sizeof *board);

	for (color = 0; color < 2; color++) {
		for (symbol = 0; symbol < 2; symbol++) {
			for (value = 1; value < 14; value++) {
				newcard.color = color;
				newcard.symbol = symbol;
				newcard.value = value;
				deck[(color * 2 + symbol) * 13 + value - 1] = newcard;
			}
		}
	}

	// shuffle(deck, 52)

	for (row = 0; row < 6; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = deck[row * 8 + col];
		}
	}
	for (col = 0; col < 4; col++) {
		board->columns[col][row] = deck[48 + col];
		board->collen[col] = 7;
	}
	for (col = 4; col < 8; col++) {
		board->collen[col] = 6;
	}
}


void card_str(Card card, char *str) {
	str[0] = ' ';
	if (card.value == 0) {
		str[1] = str[2] = ' ';
		return;
	}

	switch (card.value) {
		case 10: str[0] = '1'; str[1] = '0'; break;
		case 11: str[1] = 'J'; break;
		case 12: str[1] = 'Q'; break;
		case 13: str[1] = 'K'; break;
		default: str[1] = '0' + (char) card.value;
	}

	switch (card.color * 2 + card.symbol) {
		case 0: str[2] = 'S'; break;
		case 1: str[2] = 'C'; break;
		case 2: str[2] = 'H'; break;
		case 3: str[2] = 'D'; break;
	}
}


void board_show(Board *board) {
	int row, col;
	char str[4];
	str[3] = 0;

	for (col = 0; col < 4; col++) {
		card_str(board->freecell[col], str);
		printf("%s ", str);
	}
	printf("|");
	for (col = 0; col < 4; col++) {
		card_str(board->foundation[col][board->fdlen[col]], str);
		printf(" %s", str);
	}
	printf("\n---------------------------------\n");
	for (row = 0; row < 20; row++) {
		for (col = 0; col < 8; col++) {
			card_str(board->columns[col][row], str);
			printf(" %s", str);
		}
		printf("\n");
	}
}


int main(void) {
	Board board;
	board_init(&board);
	board_show(&board);
	return 0;
}
