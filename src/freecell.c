#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <time.h>
#include "stack.h"
#include "freecell.h"
#include "array.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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
		case 'l':  // freecell
			return tocard.value == 0;
		case 'f':  // foundation
			return (
				fromcard.symbol == tocard.symbol
				&& fromcard.value == tocard.value + 1
			);
		case 'c':  // column
			return (
				tocard.value == 0
				|| (
					fromcard.color != tocard.color
					&& fromcard.value == tocard.value - 1
				)
			);
		default:
			assert(false);
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


int play_cost(Board *board, Card *tocard) {
	int column;
	int topsortedcard_i;

	if (tocard >= (Card*) board->columns) {
		/* If the column is sorted, it is better when the topmost card
		   is a high card. If it is not, it is better there are not
		   many card stuck above the sorted ones and that the column
		   depth is not too high.
		*/

		column = (tocard - (Card*) board->columns) / MAXCOLEN;
		for (
			topsortedcard_i = board->colen[column] - 1;
			board->columns[column][topsortedcard_i].value
			== board->columns[column][topsortedcard_i + 1].value + 1;
			topsortedcard_i--
		);

		if (topsortedcard_i == 0) {
			return 13 - tocard->value;
		} else if (topsortedcard_i == 1) {
			return 13 - board->columns[column][1].value;
		}

		return topsortedcard_i + board->colen[column] - 2;

	} else if (tocard >= (Card*) board->foundation) {
		/* It is better not to move a card to the foundation if that
		   card value and the minimum card value of the opposite color
		   have a (non-absolute) difference above 2.

		   That is, if the foundation is 2S 2C 5H 3D, pushing the next
		   spade, club or diamond is free, pushing the next heart has
		   a cost of 3.
		*/
		return MAX(
			board->fdlen[2 * tocard->color + tocard->symbol]
			- MIN(
				board->fdlen[2 - 2 * tocard->color],
				board->fdlen[3 - 2 * tocard->color]
			), 1) - 1;
	} else {
		/* It is better when there are many freecells */
		return (
			(board->freecell[0].value != 0)
			+ (board->freecell[1].value != 0)
			+ (board->freecell[2].value != 0)
			+ (board->freecell[3].value != 0)
		) + 1;
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


Node* depth_search(Board * board, Node * currentnode, int budget) {
	int cost;
	Node *nextnode, *bestnode;
	Stack *nextmoves;
	Card *fromcard, *tocard;

	assert(stack_new(&nextmoves) == CC_OK);
	listmoves(board, nextmoves, currentnode);

	while (stack_size(nextmoves)) {
		assert(stack_pop(nextmoves, (void**) &tocard) == CC_OK);
		assert(stack_pop(nextmoves, (void**) &fromcard) == CC_OK);
		cost = play_cost(board, tocard);
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

		if (cost <= budget) {
			bestnode = depth_search(board, nextnode, budget - cost);
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
	int budget;
	long int start_time;

	if (argc == 1) {
		int seed;
		assert(getrandom(&seed, sizeof(seed), 0) != -1);
		srand(seed);
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
	for (budget = 10; node == NULL; budget *= 1.1) {
		play_cnt = 0;
		nodes_cnt = 0;
		node = depth_search(&board, &rootnode, budget);
		printf("Budget %d, Time: %ld, Play: %lld, Node: %lld\n", budget, time(NULL) - start_time, play_cnt, nodes_cnt);
		fflush(stdout);
	}

	printf("Found a solution, game budget is %d\n", budget);
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
