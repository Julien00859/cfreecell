#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "array.h"
#include "freecell.h"
#include "stack.h"


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static Card nullcard;
static char cardstr[4] = "   ";


// Stats
unsigned long long play_cnt;
unsigned long long nodes_cnt;
unsigned long int start_time;


void setcardstr(Card card) {
	cardstr[0] = ' ';
	if (card.value == 0) {
		cardstr[1] = cardstr[2] = ' ';
		return;
	}

	switch (card.value) {
		case 10: cardstr[0] = '1'; cardstr[1] = '0'; break;
		case 11: cardstr[1] = 'J'; break;
		case 12: cardstr[1] = 'Q'; break;
		case 13: cardstr[1] = 'K'; break;
		default: cardstr[1] = '0' + (char) card.value;
	}

	switch (card.color * 2 + card.symbol) {
		case 0: cardstr[2] = 'S'; break;
		case 1: cardstr[2] = 'C'; break;
		case 2: cardstr[2] = 'H'; break;
		case 3: cardstr[2] = 'D'; break;
	}
}


void listmoves(Board *board, Stack *nextmoves, Node *node) {
	int fromcol, tocol, symbol;
	Card *fromcard, *tocard;

	// Search for immediate moves first
	// From freecell to foundation
	for (fromcol = 0; fromcol < 4; fromcol++) {
		fromcard = &(board->freecell[fromcol]);
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f') && check_immediate(board, fromcard)) {
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard + 1) == CC_OK);
		}
	}
	// From column to foundation
	for (fromcol = 0; fromcol < 8; fromcol++) {
		fromcard = &(board->columns[fromcol][board->colen[fromcol] - 1]);
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f') && check_immediate(board, fromcard)) {
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard + 1) == CC_OK);
		}
	}
	if (stack_size(nextmoves)) return;

	// Search for any valid move then
	// From freecell...
	for (fromcol = 0; fromcol < 4; fromcol++) {
		fromcard = &(board->freecell[fromcol]);

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c') && (fromcard != node->lasttocard || tocard != node->lastfromcard)) {
				assert(stack_push(nextmoves, fromcard) == CC_OK);
				assert(stack_push(nextmoves, tocard + 1) == CC_OK);
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard + 1) == CC_OK);
		}
	}

	// From column...
	for (fromcol = 0; fromcol < 8; fromcol++) {
		fromcard = &(board->columns[fromcol][board->colen[fromcol] - 1]);

		// ...to freecell
		for (tocol = 0; tocol < 4; tocol++) {
			tocard = &(board->freecell[tocol]);
			if (validate_move(*fromcard, *tocard, 'l') && (fromcard != node->lasttocard || tocard != node->lastfromcard)) {
				assert(stack_push(nextmoves, fromcard) == CC_OK);
				assert(stack_push(nextmoves, tocard) == CC_OK);
				break;
			}
		}

		// ...to column
		for (tocol = 0; tocol < 8; tocol++) {
			if (fromcol == tocol)
				continue;
			if (board->colen[fromcol] == 2 && board->colen[tocol] == 1)
				continue;

			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c') && (fromcard != node->lasttocard || tocard != node->lastfromcard)) {
				assert(stack_push(nextmoves, fromcard) == CC_OK);
				assert(stack_push(nextmoves, tocard + 1) == CC_OK);
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f')) {
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard + 1) == CC_OK);
		}
	}
}




Node* search(Board * board, Node * currentnode) {
	return NULL;
}

int main(int argc, char *argv[]) {
	Board board;
	Node rootnode;
	Node *leaf;

	board_init(&board);

	if (argc == 1) {
		int seed;
		assert(getrandom(&seed, sizeof(seed), 0) != -1);
		srand(seed);
		board_deal(&board);
		printf("Seed: %d\n\n", seed);
	} else if (argc == 2) {
		srand(atoi(argv[1]));
		board_deal(&board);
		printf("Seed: %s\n\n", argv[1]);
	} else if (argc == 3) {
		board_load(&board, argv[2]);
		printf("File: %s\n\n", argv[2]);
	}

	board_show(&board);
	return 0;
	start_time = time(NULL);

	rootnode.parent = NULL;
	rootnode.lastfromcard = NULL;
	rootnode.lasttocard = NULL;

	leaf = search(&board, &rootnode);
	solution_show(leaf);
	stats_show();
	return 0;
}
