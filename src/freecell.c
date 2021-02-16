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
	int col;
	Card newcard;

	nullcard.value = 0;
	nullcard.symbol = 0;
	nullcard.color = 0;
	nullcard._padding = 0;

	memset(board, 0, sizeof(Board));

	for (col = 0; col < 4; col++) {
		newcard.color = col / 2;
		newcard.symbol = col % 2;
		newcard.value = 0;
		board->foundation[col][0] = newcard;
		board->fdlen[col] = 1;
	}
}


void board_deal(Board *board) {
	int row, col;
	int symbol, color, value;
	Card newcard;
	Card deck[52];

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

void board_load(Board *board, const char *pathname) {
	int fd, row, col;
	char line[32];
	Card newcard;

	fd = open(pathname, O_RDONLY);
	assert(fd > 2);
	newcard._padding = 0;
	for (row = 0; row < 7; row++) {
		assert(read(fd, line, 32));
		for (col = 0; col < (row == 6 ? 4 : 8); col++) {
			switch (line[col * 4 + 1]) {
				case 'A':
				case '1': newcard.value = 1; break;
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9': newcard.value = line[col * 4 + 1] - '0'; break;
				case '0': newcard.value = 10; break;
				case 'J': newcard.value = 11; break;
				case 'Q': newcard.value = 12; break;
				case 'K': newcard.value = 13; break;
				default: assert(0);
			}
			switch (line[col * 4 + 2]) {
				case 'S': newcard.color = 0; newcard.symbol = 0; break;
				case 'C': newcard.color = 0; newcard.symbol = 1; break;
				case 'H': newcard.color = 1; newcard.symbol = 0; break;
				case 'D': newcard.color = 1; newcard.symbol = 1; break;
				default: assert(0);
			}
			board->columns[col][row] = newcard;
		}
	}
	assert(close(fd) == 0);
	for (col = 0; col < 4; col++) {
		board->colen[col] = 6;
	}
	for (col = 4; col < 8; col++) {
		board->colen[col] = 7;
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


bool check_immediate(Board *board, Card *fromcard) {
	return fromcard->value <= MIN(
		board->fdlen[(1 - fromcard->color) * 2 + 0],
		board->fdlen[(1 - fromcard->color) * 2 + 1]
	) + 1;  // +2 but there is a nullcard on top
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
