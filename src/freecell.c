#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "stack.h"
#include "freecell.h"
#include "array.h"



static Card nullcard;
static char cardstr[4] = "   ";


// Stats
unsigned long long play_cnt;
unsigned long long nodes_cnt;


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

void shuffle(Card *deck, int len) {
	int i, r;
	Card tmp;

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


void board_show(Board *board) {
	int row, col;

	for (col = 0; col < 4; col++) {
		setcardstr(board->freecell[col]);
		printf("%s ", cardstr);
	}
	printf("|");
	for (col = 0; col < 4; col++) {
		setcardstr(board->foundation[col][board->fdlen[col]-1]);
		printf(" %s", cardstr);
	}
	printf("\n---------------------------------\n");
	for (row = 0; row < 9; row++) {
		for (col = 0; col < 8; col++) {
			setcardstr(board->columns[col][row]);
			printf(" %s", cardstr);
		}
		printf("\n");
	}
}

bool isgameover(Board *board) {
	return (
		board->fdlen[0] == 14
		&& board->fdlen[1] == 14
		&& board->fdlen[2] == 14
		&& board->fdlen[3] == 14
	);
}


void listmoves(Board *board, Stack *nextmoves, Node *node) {
	int fromcol, tocol, symbol;
	Card *fromcard, *tocard;

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
		if (validate_move(*fromcard, *tocard, 'f') && (fromcard != node->lasttocard || tocard != node->lastfromcard)) {
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
			tocard = &(board->columns[tocol][board->colen[tocol] - 1]);
			if (validate_move(*fromcard, *tocard, 'c') && (fromcard != node->lasttocard || tocard != node->lastfromcard)) {
				assert(stack_push(nextmoves, fromcard) == CC_OK);
				assert(stack_push(nextmoves, tocard + 1) == CC_OK);
			}
		}

		// ...to foundation
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (validate_move(*fromcard, *tocard, 'f') && (fromcard != node->lasttocard || tocard != node->lastfromcard)) {
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard + 1) == CC_OK);
		}
	}
}


void play(Board *board, Card *card1, Card *card2) {
	play_cnt++;

	// Update depth of column and of foundation
	if (card1 >= (Card*) board->columns) {
		board->colen[(card1 - (Card*) board->columns) / MAXCOLEN]--;
	} else if (card1 >= (Card*) board->foundation) {
		board->fdlen[(card1 - (Card*) board->foundation) / MAXFDLEN]--;
	} else {
	}

	if (card2 >= (Card*) board->columns) {
		board->colen[(card2 - (Card*) board->columns) / MAXCOLEN]++;
	} else if (card2 >= (Card*) board->foundation) {
		board->fdlen[(card2 - (Card*) board->foundation) / MAXFDLEN]++;
	} else {
	}

	// Move the card
	*card2 = *card1;
	*card1 = nullcard;
}


Node* depth_search(Board * board, Node * currentnode, int depth) {
	Node *nextnode, *bestnode;
	Stack *nextmoves;
	Card *fromcard, *tocard;

	assert(stack_new(&nextmoves) == CC_OK);
	listmoves(board, nextmoves, currentnode);

	while (stack_size(nextmoves)) {
		assert(stack_pop(nextmoves, (void**) &tocard) == CC_OK);
		assert(stack_pop(nextmoves, (void**) &fromcard) == CC_OK);
		play(board, fromcard, tocard);

		nextnode = (Node*) calloc(1, sizeof(Node));
		assert(nextnode != NULL);
		nextnode->parent = currentnode;
		nextnode->lastfromcard = fromcard;
		nextnode->lasttocard = tocard;
		nodes_cnt++;

		if (isgameover(board)) {
			return nextnode;
		}

		if (depth) {
			bestnode = depth_search(board, nextnode, depth - 1);
			if (bestnode != NULL) {
				return bestnode;
			}
		}

		free(nextnode);
		play(board, tocard, fromcard);
	}

	stack_destroy(nextmoves);
	return NULL;
}

int main(int argc, char *argv[]) {
	Board board;
	Node rootnode;
	Node *node, *nextnode;
	int depth;
	long int start_time;

	if (argc == 1) {
		srand(time(0));
	} else {
		srand(atoi(argv[1]));
	}

	// Initialization
	board_init(&board);
	board_show(&board);
	start_time = time(NULL);

	rootnode.parent = NULL;
	rootnode.lastfromcard = NULL;
	rootnode.lasttocard = NULL;

	// Search, iterative deeping
	node = NULL;
	for (depth = 1; node == NULL; depth++) {
		play_cnt = 0;
		nodes_cnt = 0;
		node = depth_search(&board, &rootnode, depth);
		printf("Depth %d, Time: %ld, Play: %lld, Node: %lld\n", depth, time(NULL) - start_time, play_cnt, nodes_cnt);
		fflush(stdout);
	}

	printf("Found a solution, game depth is %d\n", depth);
	while (node->parent != NULL) {
		play(&board, node->lasttocard, node->lastfromcard);
		setcardstr(*(node->lastfromcard));
		printf("%s -> ", cardstr);
		if (node->lasttocard < (Card*) board.foundation)
			setcardstr(*(node->lasttocard));
		else
			setcardstr(*(node->lasttocard-1));
		printf("%s\n", cardstr);
		nextnode = node;
		node = node->parent;
		free(nextnode);
	}
	return 0;
}
