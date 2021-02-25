#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "board.h"
#include "stack.h"
#include "strategy.h"

/**
 * The "Rule of Two" is a freecell property that indicates a card can be
 * automatically moved to the foundation.
 *
 * TL;DR, the card can be moved to the foundation if its value is lower or
 * equal to the card of the opposite color least value + 2.
 *
 * The card can be moved if it is the card of the least (or equal to the
 * least) value still in the cascades or the freecell. All lower cards
 * are on the foundation already and it is impossible to move a card back
 * from the foundation to the freecells. It is impossible to add cards
 * bellow the considered one so it is safe to move it to the foundation.
 *
 * The card can be moved if all the cards of the opposite color whose
 * values are strictly bellow the considered card's value - 1 are in the
 * foundation already. All cards of opposite color whose value is exactly
 * this card's value - 1 can be automatically moved to the foundation by
 * the first property of the "Rule of Two".
 */
bool respect_rule_of_two(Board *board, Card fromcard) {
	return fromcard.value <= MIN(
		board->fdlen[(1 - fromcard.color) * 2 + 0],
		board->fdlen[(1 - fromcard.color) * 2 + 1]
	) + 1;  // +2 but there is a nullcard on top
}

/**
 * It is always possible to move the cards with the least value in the
 * cascades and freecells to the foundation.
 */
void strat_auto_move(Board *board, Goal *goal) {
	int fromcol, symbol;
	Card *fromcard, *tocard;

	// From freecell to foundation
	for (fromcol = 0; fromcol < 4; fromcol++) {
		fromcard = &(board->freecell[fromcol]);
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (is_move_valid(*fromcard, *tocard, 'f') && respect_rule_of_two(board, *fromcard)) {
			assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
			assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
			move(board, fromcard, tocard + 1);
		}
	}

	// From column to foundation
	for (fromcol = 0; fromcol < 8; fromcol++) {
		fromcard = &(board->columns[fromcol][board->colen[fromcol] - 1]);
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (is_move_valid(*fromcard, *tocard, 'f') && respect_rule_of_two(board, *fromcard)) {
			assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
			assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
            move(board, fromcard, tocard + 1);
		}
	}

	if (stack_size(goal->nextmoves)) {
		goal->strat = STRAT_AUTO;
	}
}


int comp_buildfactor(const void *p1, const void *p2, const void *arg) {
    int col1, col2;
    Board *board;

    col1 = *((int*)p1);
    col2 = *((int*)p2);
    board = (Board*) arg;

	if (board->buildfactor[col1] > board->buildfactor[col2])
		return 1;
	else if (board->buildfactor[col1] < board->buildfactor[col2])
		return -1;
	return 0;
}


void strat_build_nonempty(Board *board, Goal *goal) {
	int i, j, tocol, fromcol, depth;
	Card *tocard, *fromcard, *highcard;
	int columns[] = {0, 1, 2, 3, 4, 5, 6, 7};
	Stack * nextmoves;


	qsort_r(columns, 8, sizeof(int), comp_buildfactor, board);

	for (i = goal->a; i >= 0; i--) {  // i = 7, highest build factor
		tocol = columns[i];
		if (is_empty(board, tocol)) continue;
		tocard = bottom_card(board, tocol);

		for (fromcol = goal->b; fromcol < 4; fromcol++) {  // fromcol = 0
			fromcard = &(board->freecell[fromcol]);
			if (is_move_valid(*fromcard, *tocard, 'c')) {
				assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
				assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
				move(board, fromcard, tocard + 1);
				goal->strat = STRAT_BUILD_NONEMPTY;
				goal->a = i;
				goal->b = fromcol + 1;
				goal->c = 0;
				return;
			}
		}

		for (j = goal->c; j < i; j++) {  // j = 0, lowest build factor
			fromcol = columns[j];
			if (is_empty(board, fromcol)) continue;

			depth = supermove_depth(board, fromcol, tocol);
			if (!depth) continue;

			if (supermove(board, fromcol, tocol, depth, goal->nextmoves)) {
				goal->strat = STRAT_BUILD_NONEMPTY;
				goal->a = i;
				goal->b = 4;
				goal->c = j + 1;
				return;
			}
		}
	}
}


int comp_highest_sorted_card(const void *p1, const void *p2, const void *arg) {
    int card_value1, card_value2;
    Board *board;

    board = (Board*) arg;
    card_value1 = highest_sorted_card(board, *((int*)p1))->value;
    card_value2 = highest_sorted_card(board, *((int*)p2))->value;

    if (card_value1 > card_value2)
        return 1;
    else if (card_value1 < card_value2)
        return -1;
    return 0;
}

void strat_build_empty(Board *board, Goal *goal) {
	int fromcol, tocol, columns_length, i;
	int columns[8];

	// J'AI OUBLIE LES FREECELL

    columns_length = 0;
	for (tocol = 0; tocol < 8; tocol++) {
		if (!is_empty(board, tocol)) continue;
		for (fromcol = 0; fromcol < 8; fromcol++) {
			if (is_empty(board, fromcol)) continue;
			if (is_fully_sorted(board, fromcol)) continue;
			columns[columns_length++] = fromcol;
		}
        qsort_r(columns, columns_length, sizeof(int), comp_highest_sorted_card, board);

		for (i = MIN(goal->a, columns_length - 1); i >= 0; i--) {
		    fromcol = columns[i];
		    if (supermove(board, fromcol, tocol, board->sortdepth[fromcol], goal->nextmoves)) {
                goal->strat = STRAT_BUILD_EMPTY;
		        goal->a = i + 1;
                return;
		    }
		}
	}
}


void strat_access_low_card(Board *board, Goal *goal) {
	int suit, minvalue;

	for (suit = 0; suit < 4; suit++) {

	}
}
