#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "xxhash.h"
#include "stack.h"
#include "treeset.h"
#include "freecell.h"


static Card nullcard;
static char str[4];

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

	nullcard.value = 0;
	nullcard.symbol = 0;
	nullcard.color = 0;
	nullcard._padding = 0;

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
				newcard._padding = 0;
				deck[(color * 2 + symbol) * 13 + value - 1] = newcard;
			}
		}
	}
	shuffle(deck, 52);

	// Assign each card to a column
	for (col = 0; col < 8; col++) {
		board->columns[col][0] = nullcard;
	}
	for (row = 1; row < 7; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = deck[(row - 1)* 8 + col];
		}
	}
	for (col = 0; col < 4; col++) {
		board->columns[col][row] = deck[48 + col];
		board->colen[col] = 8;
	}
	for (col = 4; col < 8; col++) {
		board->colen[col] = 7;
	}
	for (row = 8; row < MAXCOLEN; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = nullcard;
		}
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
	str[3] = 0;

	for (col = 0; col < 4; col++) {
		card_str(board->freecell[col], str);
		printf("%s ", str);
	}
	printf("|");
	for (col = 0; col < 4; col++) {
		card_str(board->foundation[col][board->fdlen[col]-1], str);
		printf(" %s", str);
	}
	printf("\n---------------------------------\n");
	for (row = 0; row < MAXCOLEN; row++) {
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

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c')) {
				stack_push(nextmoves, fromcard);
				stack_push(nextmoves, tocard + 1);
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			stack_push(nextmoves, fromcard);
			stack_push(nextmoves, tocard + 1);
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

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			if (fromcol == tocol)
				continue;
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c')) {
				stack_push(nextmoves, fromcard);
				stack_push(nextmoves, tocard + 1);
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			stack_push(nextmoves, fromcard);
			stack_push(nextmoves, tocard + 1);
		}
	}
}


void play(Board *board, Card *card1, Card *card2) {
	/*Card *p_tmp;

	// Ensure card1 is the one moving on the free space
	if (card1->value == nullcard.value) {
		p_tmp = card1;
		card1 = card2;
		card2 = p_tmp;
	}*/

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

	// Move the card
	*card2 = *card1;
	*card1 = nullcard;
}


bool explore(Board * board, TreeSet * visited_boards, Node * rootnode) {
	Node *node, *nextnode;
	Card *fromcard, *tocard;
	XXH64_hash_t *hash;

	node = rootnode;

	while (true) {
		if (stack_size(node->nextmoves)) {
			stack_pop(node->nextmoves, (void**)&tocard);
			stack_pop(node->nextmoves, (void**)&fromcard);

			play(board, fromcard, tocard);
			hash = (XXH64_hash_t*) calloc(1, sizeof(XXH64_hash_t));
			*hash = XXH64(board, sizeof(Board), 0);
			if (treeset_contains(visited_boards, hash)) {
				free(hash);
				play(board, tocard, fromcard);
				continue;
			}
			treeset_add(visited_boards, hash);

			if (isgameover(board)) {
				printf("Found a solution\n");
				play(board, tocard, fromcard);
				card_str(*fromcard, str);
				printf("%s -> ", str);
				card_str(*(tocard-1), str);
				printf("%s\n", str);

				while (node != rootnode) {
					play(board, node->lasttocard, node->lastfromcard);
					card_str(*(node->lastfromcard), str);
					printf("%s -> ", str);
					if (node->lasttocard < (Card*)board->foundation)
						card_str(*(node->lasttocard), str);
					else
						card_str(*(node->lasttocard-1), str);
					printf("%s\n", str);

					stack_destroy(node->nextmoves);
					nextnode = node;
					node = node->parent;
					free(nextnode);
				}
				return true;
			}

			nextnode = (Node*) calloc(1, sizeof(Node));
			nextnode->parent = node;
			nextnode->lastfromcard = fromcard;
			nextnode->lasttocard = tocard;
			stack_new(&(nextnode->nextmoves));
			listmoves(board, nextnode->nextmoves);
			node = nextnode;
			continue;
		}

		if (node == rootnode) {
			return false;
		}

		play(board, node->lasttocard, node->lastfromcard);
		stack_destroy(node->nextmoves);
		nextnode = node;
		node = node->parent;
		free(nextnode);
	}
}


int board_comp(const void * ptr_h1, const void * ptr_h2) {
	XXH64_hash_t h1, h2;

	h1 = *(XXH64_hash_t*) ptr_h1;
	h2 = *(XXH64_hash_t*) ptr_h2;

	if (h1 < h2) return -1;
	if (h2 < h1) return 1;
	return 0;
}


int main(void) {
	Board board;
	board_init(&board);
	board_show(&board);

	TreeSet *visited_boards;
	treeset_new(board_comp, &visited_boards);

	Node rootnode;
	rootnode.parent = NULL;
	rootnode.lastfromcard = NULL;
	rootnode.lasttocard = NULL;
	stack_new(&(rootnode.nextmoves));
	listmoves(&board, rootnode.nextmoves);

	explore(&board, visited_boards, &rootnode);

	stack_destroy(rootnode.nextmoves);
	treeset_foreach(visited_boards, free);
	treeset_destroy(visited_boards);
	return 0;
}
