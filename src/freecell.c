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

	memset(board, 0, sizeof *board);

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
	Card **cardpair;

	// From freecell...
	for (fromcol = 0; fromcol < 4; fromcol++) {
		fromcard = &(board->freecell[fromcol]);

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			cardpair = calloc(2, sizeof(Card*));
			if (cardpair == NULL) {
				fprintf(stderr, "calloc fail\n");
				exit(1);
			}
			cardpair[0] = fromcard;
			cardpair[1] = tocard + 1;
			stack_push(nextmoves, cardpair);
		}

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c')) {
				cardpair = calloc(2, sizeof(Card*));
				if (cardpair == NULL) {
					fprintf(stderr, "calloc fail\n");
					exit(1);
				}
				cardpair[0] = fromcard;
				cardpair[1] = tocard + 8;
				stack_push(nextmoves, cardpair);
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
				cardpair = calloc(2, sizeof(Card*));
				if (cardpair == NULL) {
					fprintf(stderr, "calloc fail\n");
					exit(1);
				}
				cardpair[0] = fromcard;
				cardpair[1] = tocard;
				stack_push(nextmoves, cardpair);
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			cardpair = calloc(2, sizeof(Card*));
			if (cardpair == NULL) {
				fprintf(stderr, "calloc fail\n");
				exit(1);
			}
			cardpair[0] = fromcard;
			cardpair[1] = tocard + 1;
			stack_push(nextmoves, cardpair);
		}

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			if (fromcol == tocol)
				continue;
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c')) {
				cardpair = calloc(2, sizeof(Card*));
				if (cardpair == NULL) {
					fprintf(stderr, "calloc fail\n");
					exit(1);
				}
				cardpair[0] = fromcard;
				cardpair[1] = tocard + 8;
				stack_push(nextmoves, cardpair);
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
	Card *nextmove[2];
	size_t freeze;
	char str[4];
	str[3] = 0;

	if (isgameover(board))
		return true;

	stack_new(&nextmoves);
	listmoves(board, nextmoves);
	while (stack_size(nextmoves)) {
		stack_pop(nextmoves, &nextmove);
		play(board, nextmove[0], nextmove[1]);

		freeze = hash(board);
		if (!hashset_contains(visited_boards, freeze)) {
			hashset_add(visited_boards, freeze);
			if (explore(board, visited_boards)) {
				card_str(*nextmove[0], str);
				printf("%s -> ", str);
				card_str(*nextmove[1], str);
				printf("%s\n", str);
				return true;
			}
		}

		play(board, nextmove[1], nextmove[0]);
	}
	return false;
}


/*  The hashtable_add uses the following code:
 *
enum cc_stat hashtable_add(HashTable *table, void *key, void *val)
{
    const size_t hash = table->hash(key, table->key_len, table->hash_seed);
    const size_t i    = hash & (table->capacity - 1);
    ...
*/

/* The xxhash function has the following signature (from README.md):
 *
 *  	XXH64_hash_t hash = XXH64(buffer, size, seed);
 *
 * The hash function used by hashtable has the following signature:
 *
 *  	struct hashtable_s {
 *  		size_t (*hash) (const void *key, int l, uint32_t seed);
 *  	}
 */




int main(void) {
	Board board;
	HashSet *visited_boards;

	nullcard.value = 0;

	// Should reconfigure the hash function to xxh64
	hashset_new(&visited_boards);
	visited_boards->table->hash = xxh64;
	visited_boards->table->key_len = 4 + 4 * MAXFDEN + 8 * MAXCOLEN;

	board_init(&board);
	board_show(&board);

	explore(&board, visited_boards);

	return 0;
}
