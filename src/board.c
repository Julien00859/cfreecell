#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "board.h"

int count_freecell(Board *board) {
	int freecell_cnt, col;

	freecell_cnt = 0;
	for (col = 0; col < 4; col++) {
		freecell_cnt += is_nullcard(board->freecell[col]);
	}

	return freecell_cnt;
}

int count_empty_column(Board *board) {
	int empty_column_cnt, col;

	empty_column_cnt = 0;
	for (col = 0; col < 8; col++) {
		empty_column_cnt += is_empty(board, col);
	}

	return empty_column_cnt;
}


/**
 * Determines whether we can move the ``fromcard`` on the ``tocard`` given
 * where (`l` freecell, `f` foundation, `c` columns) the ``tocard`` is.
 */
bool is_move_valid(Card fromcard, Card tocard, char destination) {
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

/**
 * Determines whether the specified card is nullcard.
 */
bool is_nullcard(Card card) {
	return card.value == nullcard.value;
}

/**
 * Determines whether the specified column is empty.
 */
bool is_empty(Board *board, int col) {
	return board->colen[col] == 1;
}

/**
 * Determines if the specified column is fully sorted.
 */
bool is_fully_sorted(Board *board, int col) {
	return board->sortdepth[col] == board->colen[col] - 1;
}

/**
 * Determines if the game is won.
 */
bool is_game_won(Board *board) {
	return (
		board->fdlen[0] == 14
		&& board->fdlen[1] == 14
		&& board->fdlen[2] == 14
		&& board->fdlen[3] == 14
	);
}

/**
 * Determines if two card are equals
 */
bool are_card_equal(Card c1, Card c2) {
    if (c1.value != c2.value) return false;
    if (c1.symbol != c2.symbol) return false;
    if (c1.color != c2.color) return false;
    return true;
}

/**
 * Get the lowest card of the specified column.
 */
Card* bottom_card(Board *board, int col) {
	return &(board->columns[col][board->colen[col] - 1]);
}

/**
 * Get the highest sorted card we can move from the specified column.
 */
Card* highest_sorted_card(Board *board, int col) {
	return &(board->columns[col][board->colen[col] - board->sortdepth[col]]);
}

/**
 * Searches for a precise card in the cascades
 */
CardPosPair search_card(Board *board, Card searched_card) {
    int col, row;
    Card *card;
    CardPosPair cpp;

    memset(&cpp, 0, sizeof(CardPosPair));

    for (col = 0; col < 4; col++) {
        if (!are_card_equal(board->freecell[col], searched_card)) continue;
        cpp.col = col;
        cpp.row = MAXCOLEN;
        return cpp;
    }

    for (col = 0; col < 8; col++) {
        for (row = 1, card = bottom_card(board, col); !is_nullcard(*card); row++, card--) {
            if (!are_card_equal(searched_card, *card)) continue;
            cpp.col = col;
            cpp.row = board->colen[col] - row;
            return cpp;
        }
    }
    assert(false);
}

/**
 * Compute how many cards are sorted (by the solitaire rule) from the
 * bottom of each column.
 */
void compute_sortdepth(Board *board) {
	int col;
    for (col = 0; col < 8; col++) {
        compute_sortdepth_col(board, col);
    }
}

void compute_sortdepth_col(Board *board, int col) {
    int depth;
    Card *card;

    card = bottom_card(board, col);
    depth = !is_nullcard(*card);
    while(!is_nullcard(*(card - 1)) && is_move_valid(*card, *(card - 1), 'c')) {
        depth++;
        card--;
    }
    board->sortdepth[col] = depth;
}

/**
 * Compute the "build factor" of each column, the "build factor" indicates
 * how appropriate it is to add cards on the column. It is better (higher
 * bf value) to build on fully sorted columns, it is then better to not
 * build on columns where a lot of low value cards are blocked.
 */
void compute_buildfactor(Board *board) {
	int col, row;
	Card *highcard, *card;

	for (col = 0; col < 8; col++) {
		if (is_empty(board, col)) {
			board->buildfactor[col] = 0;
		} else if (is_fully_sorted(board, col)) {
			board->buildfactor[col] = board->columns[col][1].value * MAXCOLEN + board->colen[col];
		} else {
			board->buildfactor[col] = -board->colen[col];
			highcard = highest_sorted_card(board, col);
			for (card = highcard; !is_nullcard(*card); card--) {
				if (card->value < highcard->value) {
					board->buildfactor[col] -= (highcard->value - card->value);
				} else {
					highcard = card;
				}
			}
		}
	}
}

/**
 * Shuffle a deck of card.
 */
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

/**
 * Initializes an empty board.
 */
void board_init(Board *board) {
	int col;
	Card newcard;

	nullcard.value = 0;
	nullcard.symbol = 0;
	nullcard.color = 0;
	nullcard._padding = 0;

	memset(board, 0, sizeof(Board));

	for (col = 0; col < 8; col++) {
		board->columns[col][0] = nullcard;
		board->colen[col] = 1;
	}
	for (col = 0; col < 4; col++) {
		newcard.color = col / 2;
		newcard.symbol = col % 2;
		newcard.value = 0;
		board->foundation[col][0] = newcard;
		board->fdlen[col] = 1;
	}
}

/**
 * Randomly deal a board.
 */
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

/**
 * Load a board from an ascii text file.
 */
void board_load(Board *board, const char *pathname) {
	int fd, row, col;
	char line[32];
	Card newcard;

	fd = open(pathname, O_RDONLY);
	assert(fd > 2);
	newcard._padding = 0;
	for (row = 1; row < 8; row++) {
		assert(read(fd, line, 32));
		for (col = 0; col < (row == 7 ? 4 : 8); col++) {
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
		board->colen[col] = 8;
	}
	for (col = 4; col < 8; col++) {
		board->colen[col] = 7;
	}
}

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

/**
 * Print the board on screen.
 */
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
	for (row = 0; row < 10; row++) {
		for (col = 0; col < 8; col++) {
			setcardstr(board->columns[col][row]);
			printf(" %s", cardstr);
		}
		printf("\n");
	}
}

/**
 * Swap a card and a nullcard on the board. The function is reversible.
 */
void move(Board *board, Card *card1, Card *card2) {
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

void humanmove(Board *board, int fromcol, int tocol) {
    int suit;
    Card *fromcard, *tocard;

    if (fromcol >= 10) fromcard = &(board->freecell[fromcol - 10]);
    else fromcard = bottom_card(board, fromcol - 1);

    suit = fromcard->color * 2 + fromcard->symbol;
    if (tocol == 0) tocard = &(board->foundation[suit][board->fdlen[suit]]);
    else if (tocol >= 10) tocard = &(board->freecell[tocol-10]);
    else tocard = bottom_card(board, tocol - 1) + 1;

    move(board, fromcard, tocard);
}

int supermove_depth(Board *board, int fromcol, int tocol) {
	Card *fromcard, *highcard, *tocard;

	fromcard = bottom_card(board, fromcol);
	highcard = highest_sorted_card(board, fromcol);
	tocard = bottom_card(board, tocol);

	if (highcard->value < tocard->value - 1) return 0;
	if (fromcard->value > tocard->value - 1) return 0;
	if (tocard->color == highcard->color) {
		   if ((tocard->value & 1) != (highcard->value & 1)) return 0;
	} else if ((tocard->value & 1) == (highcard->value & 1)) return 0;

	return tocard->value - fromcard->value;
}


bool supermove(Board *board, int fromcol, int tocol, int card_cnt, Stack * nextmoves) {
	/* The bellow example showcases the supermove algorithm using a game
	 * with a total of 2 freecells and 4 columns whose 1 is empty. We want
	 * to move the second (C2) column on the first (C1) column.
	 *
	 *    ###########
	 *    # ___ ___ #########
	 *    #-----------------#
	 *    # Q   8   2   ___ #
	 *    # J   2   6       #
	 *    #     3           #
	 *    #    10           #
	 *    #     9           #
	 *    #     8           #
	 *    #     7           #
	 *    #     6           #
	 *    #     5           #
	 *    #     4           #
	 *    #     3           #
	 *    #     2           #
	 *    #                 #
	 *    ###################
	 *
	 * Move C2 {5, 4, 3, 2} -> C3 | 3 freecells | 2a 2b 24 23 43 b3 a3
	 *  Move C2 {8, 7, 6} -> C4   | 2 freecells | 2a 2b 24 b4 a4
	 *   Move C2 {10, 9} -> C1    | 2 freecells | 2a 21 a1
	 *  Move C4 {8, 7, 6} -> C1   | 2 freecells | 4a 4b 41 b1 a1
	 * Move C3 {5, 4, 3, 2} -> C1 | 3 freecells | 3a 3b 34 31 41 b1 a1
	 */

	int freecell, col, row;
	Card *fromcard, *tocard;
	Stack *tempmoves;

	tocard = bottom_card(board, tocol);
	fromcard = bottom_card(board, fromcol);
	freecell = count_freecell(board) + count_empty_column(board);
	if (is_empty(board, tocol)) freecell--;

	// Enough freecells to move the cards right away
	if (card_cnt <= freecell + 1) {
		assert(stack_new(&tempmoves) == CC_OK);

		// Stack as many card in the freecells as needed
		for (col = 0; col < 4 && card_cnt - 1; col++) {
			if (!is_nullcard(board->freecell[col])) continue;
			move(board, fromcard, &(board->freecell[col]));
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, &(board->freecell[col])) == CC_OK);
			assert(stack_push(tempmoves, &(board->freecell[col])) == CC_OK);
			fromcard--;
			card_cnt--;
		}

		// Stack as many card in the empty columns as needed
		for (col = 0; col < 8 && card_cnt - 1; col++) {
			if (!is_empty(board, col) || col == tocol) continue;
			move(board, fromcard, bottom_card(board, col) + 1);
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, bottom_card(board, col)) == CC_OK);
			assert(stack_push(tempmoves, bottom_card(board, col)) == CC_OK);
			fromcard--;
			card_cnt--;
		}

		// No freecell left, this card must move otherwise the move is impossible
		if (!is_move_valid(*fromcard, *tocard, 'c')) {
		    // Move impossible, undo stacking
            while (stack_size(tempmoves)) {
                fromcard++;
                stack_pop(nextmoves, NULL);
                stack_pop(nextmoves, NULL);
                stack_pop(tempmoves, (void**)&tocard);
                move(board, tocard, fromcard);
            }
            return false;
        }

		// Move the card
		tocard++;
		move(board, fromcard, tocard);
		assert(stack_push(nextmoves, fromcard) == CC_OK);
		assert(stack_push(nextmoves, tocard) == CC_OK);
		fromcard--;

		// Unstack the cards from the freecells to the dest column
		while (stack_size(tempmoves)) {
			tocard++;
			assert(stack_pop(tempmoves, (void**)&fromcard) == CC_OK);
			move(board, fromcard, tocard);
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard) == CC_OK);
		}
		stack_destroy(tempmoves);
		return true;
	}

	// Deep supermove, temporary stack some cards on a non-empty column
	for (col = 0; col < 8; col++) {
		if (is_empty(board, col) || col == tocol) continue;
		tocard = bottom_card(board, col);
		for (row = 0; row <= freecell; row++) {
			if (!is_move_valid(*(fromcard - row), *tocard, 'c')) continue;
			return _deepsupermove(board, fromcol, col, tocol, card_cnt, row + 1, nextmoves);
		}
	}

	// Deep supermove, temporary stack some cards on an empty column
	for (col = 0; col < 8; col++) {
		if (!is_empty(board, col) || col == tocol) continue;
		return _deepsupermove(board, fromcol, col, tocol, card_cnt, freecell + 1, nextmoves);
	}

	// Impossible to supermove
	return false;
}


bool _deepsupermove(Board *board, int fromcol, int tempcol, int tocol, int total_card_cnt, int card_cnt, Stack *nextmoves) {
	int i;
	Card *fromcard, *tocard;

	supermove(board, fromcol, tempcol, card_cnt, nextmoves);
	if (!supermove(board, fromcol, tocol, total_card_cnt - card_cnt, nextmoves)) {
		// Undo the first supermove
		for (i = 0; i < card_cnt * 2 - 1; i++) {
			assert(stack_pop(nextmoves, (void**)&fromcard) == CC_OK);
			assert(stack_pop(nextmoves, (void**)&tocard) == CC_OK);
			move(board, fromcard, tocard);
		}
		return false;
	}
	supermove(board, tempcol, tocol, card_cnt, nextmoves);
	return true;
}

bool superaccess(Board *board, CardPosPair cpp, Stack * nextmoves, bool use_empty) {
    int row, tocol, depth, freecell_cnt, empty_cols_cnt;
    Card *fromcard, *tocard;
    Card *empty_cols[8];
    Card *freecells[4];

    assert(!stack_size(nextmoves));

    empty_cols_cnt=0;
    for (tocol = 0; tocol < 8; tocol++)
        if (is_empty(board, tocol))
            empty_cols[empty_cols_cnt++] = &board->columns[tocol][0];
    freecell_cnt=0;
    for (tocol = 0; tocol < 4; tocol++)
        if (is_nullcard(board->freecell[tocol]))
            freecells[freecell_cnt++] = &board->freecell[tocol];

    row = board->colen[cpp.col] - 1;
    CONTINUE:;
    while (row > cpp.row) {
        // Supermove to another column
        for (tocol = 0; tocol < 8; tocol++) {
            if (is_empty(board, tocol)) continue;
            depth = supermove_depth(board, cpp.col, tocol);
            if (0 < depth && depth < row - cpp.row) {
                if (!supermove(board, cpp.col, tocol, depth, nextmoves)) continue;
                compute_sortdepth_col(board, cpp.col);
                row -= depth;
                goto CONTINUE;
            }
        }

        // Supermove to an empty column
        if (use_empty && board->sortdepth[cpp.col] > 1) {
            for (depth = MIN(supermove_depth(board, cpp.col, tocol), row - cpp.row); depth > 1; depth--) {
                if (!supermove(board, cpp.col, tocol, depth, nextmoves)) continue;
                compute_sortdepth_col(board, cpp.col);
                row -= depth;
                goto CONTINUE;
            }
        }

        // Stack on a freecell or (in last resort) an empty column
        fromcard = &board->columns[cpp.col][row];
        if (freecell_cnt || (use_empty && empty_cols_cnt)) {
            tocard = freecell_cnt ? freecells[freecell_cnt--] : empty_cols[empty_cols_cnt--];
            assert(stack_push(nextmoves, fromcard) == CC_OK);
            assert(stack_push(nextmoves, tocard) == CC_OK);
            move(board, fromcard, tocard);
            compute_sortdepth_col(board, cpp.col);
            row--;
            continue;
        }

        // Cannot unstack more cards
        break;
    }

    // Wanted card is accessible
    if (row == cpp.row)
        return true;

    // Couldn't unstack enough card, restore initial state
    while (stack_size(nextmoves)) {
        assert(stack_pop(nextmoves, (void**)&fromcard) == CC_OK);
        assert(stack_pop(nextmoves, (void**)&tocard) == CC_OK);
        move(board, fromcard, tocard);
    }
    return false;
}