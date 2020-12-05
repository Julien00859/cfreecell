#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "xxhash.h"
#include "stack.h"
#include "treetable.h"
#include "freecell.h"


static Card nullcard;
static char cardstr[4] = "   ";
static unsigned long play_cnt = 0;


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
	for (row = 0; row < MAXCOLEN; row++) {
		for (col = 0; col < 8; col++) {
			setcardstr(board->columns[col][row]);
			printf(" %s", cardstr);
		}
		printf("\n");
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


int board_comp(const void * ptr_n1, const void * ptr_n2) {
	Node n1, n2;

	n1 = *(Node*) ptr_n1;
	n2 = *(Node*) ptr_n2;

	// Sort games by score
	if (n1.score < n2.score) return -1;
	if (n1.score > n2.score) return 1;

	// Sub-sort by hash
	// Is it necessary ? Could scores be == ?
	if (n1.hash < n2.hash) return -1;
	if (n1.hash > n2.hash) return 1;

	return 0;
}


double evaluate(Board * board) {
	return 1.0;
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


void replay(Board * board, Node * fromnode, Node * tonode) {
	Stack * cardstack;
	Card *fromcard, *tocard;

	stack_new(&cardstack);
	while (fromnode->depth > tonode->depth) {
		play(board, fromnode->lasttocard, fromnode->lastfromcard);
		fromnode = fromnode->parent;
	}
	while (fromnode->depth < tonode->depth) {
		stack_push(cardstack, tonode->lastfromcard);
		stack_push(cardstack, tonode->lasttocard);
		tonode = tonode->parent;
	}
	while (fromnode != tonode) {
		play(board, fromnode->lasttocard, fromnode->lastfromcard);
		stack_push(cardstack, tonode->lastfromcard);
		stack_push(cardstack, tonode->lasttocard);
		tonode = tonode->parent;
		fromnode = fromnode->parent;
	}
	while (stack_size(cardstack)) {
		stack_pop(cardstack, (void**) &tocard);
		stack_pop(cardstack, (void**) &fromcard);
		play(board, fromcard, tocard);
	}
	stack_destroy(cardstack);
}


void depth_search(Board * board, TreeSet * boards, Node * currentnode, int depth) {
	Node *nextnode;
	Stack *nextmoves;
	Card *fromcard, *tocard;

	stack_new(&nextmoves);
	listmoves(board, nextmoves);

	while (stack_size(nextmoves)) {
		stack_pop(nextmoves, (void**) &tocard);
		stack_pop(nextmoves, (void**) &fromcard);
		play(board, fromcard, tocard);

		nextnode = (Node*) calloc(1, sizeof(Node));
		nextnode->hash = XXH64(board, sizeof(Board), 0);
		nextnode->score = evaluate(board);
		nextnode->parent = currentnode;
		nextnode->depth = currentnode->depth + 1;
		nextnode->lastfromcard = fromcard;
		nextnode->lasttocard = tocard;

		if (!treeset_contains(boards, nextnode)) {
			treeset_add(boards, nextnode);
			if (depth) {
				depth_search(board, boards, nextnode, depth - 1);
			}
		} else {
			free(nextnode);
		}

		play(board, tocard, fromcard);
	}

	stack_destroy(nextmoves);
}


bool astar_search(Board * board, TreeSet * boards, Node * rootnode) {
	Node *node, *nextnode;

	node = rootnode;

	while (!isgameover(board)) {
		treeset_get_last(boards, (void**) &nextnode);
		if (node->score == -INFINITY) {
			printf("No solution found\n");
			return false;
		}
		replay(board, node, nextnode);
		node = nextnode;
		depth_search(board, boards, node, 5);
		node->score = -INFINITY;
		treeset_remove(boards, node, NULL);
		treeset_add(boards, node);
	}

	printf("Found a solution\n");
	while (node->parent != NULL) {
		play(board, node->lasttocard, node->lastfromcard);
		setcardstr(*(node->lastfromcard));
		printf("%s -> ", cardstr);
		if (node->lasttocard < (Card*) board->foundation)
			setcardstr(*(node->lasttocard));
		else
			setcardstr(*(node->lasttocard-1));
		printf("%s\n", cardstr);
		node = node->parent;
	}
	return true;
}


int main(void) {
	Board board;
	board_init(&board);
	board_show(&board);

	Node rootnode;
	rootnode.depth = 0;
	rootnode.hash = XXH64(&board, sizeof(Board), 0);
	rootnode.score = evaluate(&board);
	rootnode.parent = NULL;
	rootnode.lastfromcard = NULL;
	rootnode.lasttocard = NULL;

	TreeSet *boards;
	treeset_new(board_comp, &boards);
	treeset_add(boards, &rootnode);

	astar_search(&board, boards, &rootnode);

	printf("play() call count: %ld\n", play_cnt);

	treeset_foreach(boards, free);
	treeset_destroy(boards);
	return 0;
}
