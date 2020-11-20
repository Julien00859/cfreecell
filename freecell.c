#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef struct card {
	unsigned int color:1;
	unsigned int symbol:1;
	unsigned int value:4;
	unsigned int _fill:2;
} Card;


typedef struct board {
	Card freecell[4];
	Card foundation[4][14];
	Card columns[8][21];
	uint8_t fdlen[4];
	uint8_t collen[8];
} Board;


void shuffle(Card *deck, int len) {
	int i, r;
	Card tmp;

	srand(time(0));
	for (i = 0; i < len; i++) {
		r = rand() % len;
		tmp = deck[i];
		deck[i] = deck[r];
		deck[r] = tmp;
	}
}


void board_init(Board *board) {
	int row, col;
	int symbol, color, value;
	Card deck[52];
	Card newcard;

	memset(board, 0, sizeof *board);

	for (col = 0; col < 4; col++) {
		newcard.color = col / 2;
		newcard.symbol = col % 2;
		newcard.value = 0;
		board->foundation[col][0] = newcard;
	}

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

	shuffle(deck, 52);

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


bool validate_move(Card fromcard, Card tocard, char destination) {
	if (fromcard.value == 0)
		return false;

	switch (destination) {
		case 'l':
			return tocard.value == 0;
		case 'f':
			return (
				fromcard.symbol == tocard.symbol
				&& fromcard.value == tocard.value + 1
			);
		case 'c':
			return (
				tocard.value == 0
				|| (
					fromcard.color != tocard.color
					&& fromcard.value == tocard.value - 1
				)
			);
		default:
			exit(1);
	}
}


bool printmove(Card fromcard, Card tocard, char dest) {
	char str[] = " _  >  _ ";
	if (validate_move(fromcard, tocard, dest)) {
		card_str(fromcard, str);
		card_str(tocard, str+6);
		printf("%s\n", str);
		return true;
	}
	return false;
}


void listmoves(Board *board) {
	int fromcol, tocol, symbol;
	Card fromcard, tocard;

	// From freecell...
	for (fromcol = 0; fromcol < 4; fromcol++) {
		fromcard = board->freecell[fromcol];

		// ...to foundation
		symbol = fromcard.color * 2 + fromcard.symbol;
		tocard = board->foundation[symbol][board->fdlen[symbol] - 1];
		printmove(fromcard, tocard, 'f');

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			tocard = board->columns[tocol][board->collen[tocol] - 1];
			printmove(fromcard, tocard, 'c');
		}
	}

	// From column...
	for (fromcol = 0; fromcol < 8; fromcol++) {
		fromcard = board->columns[fromcol][board->collen[fromcol] - 1];

		// ...to freecell
		for (tocol = 0; tocol < 4; tocol++) {
			tocard = board->freecell[tocol];
			if (printmove(fromcard, tocard, 'l'))
				break;
		}

		// ...to foundation
		symbol = fromcard.color * 2 + fromcard.symbol;
		tocard = board->foundation[symbol][board->fdlen[symbol] - 1];
		printmove(fromcard, tocard, 'f');

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			if (fromcol == tocol)
				continue;
			tocard = board->columns[tocol][board->collen[tocol] - 1];
			printmove(fromcard, tocard, 'c');
		}
	}
}


int main(void) {
	Board board;
	board_init(&board);
	board_show(&board);
	listmoves(&board);
	return 0;
}
