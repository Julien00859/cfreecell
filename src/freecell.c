#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "xxhash.h"
#include "stack.h"
#include "hashset.h"
#include "freecell.h"


static Card nullcard;

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

	memset(board, 0, sizeof(Board));

	// Init foundation
	for (col = 0; col < 4; col++) {
		newcard.color = col / 2;
		newcard.symbol = col % 2;
		newcard.value = 0;
		board->foundation[col][0] = newcard;
		board->fdlen[col] = 1;
	}

	// Create a deck of card
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

	// Assign each card to a column
	for (row = 0; row < 6; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = deck[row * 8 + col];
		}
	}
	for (col = 0; col < 4; col++) {
		board->columns[col][row] = deck[48 + col];
		board->colen[col] = 7;
	}
	for (col = 4; col < 8; col++) {
		board->colen[col] = 6;
	}

	/*for (row = 8; row < MAXCOLEN; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = nullcard;
		}
	}*/
}


bool isgameover(Board *board) {
	return (
		board->foundation[0][13].value
		&& board->foundation[1][13].value
		&& board->foundation[2][13].value
		&& board->foundation[3][13].value
	);
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
	for (row = 0; row < 9; row++) {
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

void listmoves(Board *board, Stack * nextmoves) {
	int fromcol, tocol, symbol;
	Card *fromcard, *tocard;

	// From freecell...
	for (fromcol = 0; fromcol < 4; fromcol++) {
		fromcard = &(board->freecell[fromcol]);

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			stack_push(nextmoves, fromcard);
			stack_push(nextmoves, tocard);
		}

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c')) {
				stack_push(nextmoves, fromcard);
				stack_push(nextmoves, tocard);
			}
		}
	}

	// From column...
	for (fromcol = 0; fromcol < 8; fromcol++) {
		fromcard = &(board->columns[fromcol][board->colen[fromcol] - 1]);

		// ...to freecell
		for (tocol = 0; tocol < 4; tocol++) {
			tocard = &(board->freecell[tocol]);
			if (validate_move(*fromcard, *tocard, 'l')) {
				stack_push(nextmoves, fromcard);
				stack_push(nextmoves, tocard);
				break;
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			stack_push(nextmoves, fromcard);
			stack_push(nextmoves, tocard);
		}

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			if (fromcol == tocol)
				continue;
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c')) {
				stack_push(nextmoves, fromcard);
				stack_push(nextmoves, tocard);
			}
		}
	}
}


void play(Board *board, Card *card1, Card *card2) {
	Card *p_tmp;

	// Ensure card1 is the one moving on the free space
	if (card1->value == nullcard.value) {
		p_tmp = card1;
		card1 = card2;
		card2 = p_tmp;
	}

	// Update depth of column and of foundation
	if (card1 >= (Card*)board->columns) {
		board->colen[(card1 - (Card*)board->columns) / MAXCOLEN]--;
	} else if (card1 >= (Card*)board->foundation) {
		board->fdlen[(card1 - (Card*)board->foundation) / MAXFDLEN]--;
	} else {
	}

	if (card2 >= (Card*)board->columns) {
		board->colen[(card2 - (Card*)board->columns) / MAXCOLEN]++;
	} else if (card2 >= (Card*)board->foundation) {
		board->fdlen[(card2 - (Card*)board->foundation) / MAXFDLEN]++;
	} else {
	}

	*card2 = *card1;
	*card1 = nullcard;
}


bool explore(Board * board, HashSet * visited_boards) {
	Stack *nextmoves;
	Card *fromcard, *tocard;
	char str[4];
	str[3] = 0;

	if (isgameover(board))
		return true;

	stack_new(&nextmoves);
	listmoves(board, nextmoves);
	while (stack_size(nextmoves)) {
		stack_pop(nextmoves, (void**)&tocard);
		stack_pop(nextmoves, (void**)&fromcard);
		play(board, fromcard, tocard);

		if (!hashset_contains(visited_boards, board)) {
			hashset_add(visited_boards, board);
			//board_show(board);
			if (explore(board, visited_boards)) {
				card_str(*fromcard, str);
				printf("%s -> ", str);
				card_str(*tocard, str);
				printf("%s\n", str);
				return true;
			}
		}

		play(board, tocard, fromcard);
	}
	stack_destroy(nextmoves);
	return false;
}


int board_comp(const void *b1, const void *b2) {
	return memcmp(b1, b2, sizeof(Board));
}


int main(void) {
	nullcard.value = 0;

	Board board;
	board_init(&board);
	board_show(&board);

	HashSet *visited_boards;
	HashSetConf hsc;
	hashset_conf_init(&hsc);
	hsc.hash = XXH64;
	hsc.key_compare = board_comp;
	hsc.key_length = sizeof(Card) * 4 + 4 * MAXFDLEN + 8 * MAXCOLEN;
	hashset_new_conf(&hsc, &visited_boards);

	explore(&board, visited_boards);

	hashset_destroy(visited_boards);
	return 0;
}
