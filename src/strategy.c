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
    int col1, col2, card_value1, card_value2;
    Board *board;

    board = (Board*) arg;
    col1 = *((int*)p1);
    col2 = *((int*)p2);
    card_value1 = col1 < 8 ? highest_sorted_card(board, col1)->value : board->freecell[col1 - 8].value;
    card_value2 = col2 < 8 ? highest_sorted_card(board, col2)->value : board->freecell[col2 - 8].value;

    if (card_value1 > card_value2)
        return 1;
    else if (card_value1 < card_value2)
        return -1;
    return 0;
}

/**
 * Start empty columns high (king first) skipping fully-sorted columns and partially-sorted columns when it is
 * impossible to move all the sorted cards.
 */
void strat_build_empty(Board *board, Goal *goal) {
	int fromcol, tocol, columns_length, i;
	int columns[12];  // Maximum 8 columns + 4 freecells, columns have value 0->7, freecells 8->11

    columns_length = 0;
	for (tocol = 0; tocol < 8; tocol++) {
		if (!is_empty(board, tocol)) continue;
		for (fromcol = 0; fromcol < 8; fromcol++) {
			if (is_empty(board, fromcol)) continue;
			if (is_fully_sorted(board, fromcol)) continue;
			columns[columns_length++] = fromcol;
		}
		for (fromcol = 0; fromcol < 4; fromcol++) {
		    if (is_nullcard(board->freecell[fromcol])) continue;
		    columns[columns_length++] = fromcol + 8;
		}
        qsort_r(columns, columns_length, sizeof(int), comp_highest_sorted_card, board);

		for (i = MIN(goal->a, columns_length - 1); i >= 0; i--) {
		    fromcol = columns[i];
		    if (fromcol > 8) {
                stack_push(goal->nextmoves, &board->freecell[fromcol - 8]);
                stack_push(goal->nextmoves, bottom_card(board, tocol) + 1);
                move(board, &board->freecell[fromcol - 8], bottom_card(board, tocol) + 1);
                goal->strat = STRAT_BUILD_EMPTY;
                goal->a = i - 1;
                return;
            }
		    if (supermove(board, fromcol, tocol, board->sortdepth[fromcol], goal->nextmoves)) {
                goal->strat = STRAT_BUILD_EMPTY;
                goal->a = i - 1;
                return;
            }
        }

		break;  // No need to apply the same algo to another empty column, the result would be the same
	}
}

int comp_fdlen(const void *p1, const void *p2, const void *arg) {
    int suit1, suit2;
    Board *board;

    suit1 = *((int*)p1);
    suit2 = *((int*)p2);
    board = (Board*) arg;

    if (board->fdlen[suit1] > board->fdlen[suit2])
        return 1;
    else if (board->fdlen[suit1] > board->fdlen[suit2])
        return -1;
    return 0;
}

/**
 * Find a card that is low and destruct the column to access it
 */
void strat_access_low_card(Board *board, Goal *goal) {
	int i, suit, minvalue, freecell, freecol, depth;
	int suits[4] = {0, 1, 2, 3};
	Card low_card, *card;
	CardPosPair cpp;

	qsort_r(suits, 4, sizeof(int), comp_fdlen, board);

	for (freecell = 3; freecell >= 0 && !is_nullcard(board->freecell[freecell]); freecell--);
    for (freecol = 7; freecol >= 0 && !is_empty(board, freecol); freecol--);
    if (freecell == -1 && freecol == -1) return;

	for (i = goal->a; i < 4; i++) {  // i = 0
	    suit = suits[i];
        low_card.color = suit & 0b10;
        low_card.symbol = suit & 0b01;
        low_card.value = board->fdlen[suit];

        cpp = search_card(board, low_card);

        // Low card is on freecell
        if (cpp.row == MAXCOLEN) {  // Awful hack
            stack_push(goal->nextmoves, &board->freecell[cpp.col]);
            stack_push(goal->nextmoves, &board->foundation[suit][board->fdlen[suit]]);
            move(board, &board->freecell[cpp.col], &board->foundation[suit][board->fdlen[suit]]);
            goal->strat = STRAT_ACCESS_LOW_CARD;
            goal->a = i + 1;
            goal->b = 0;
            return;
        }

        // Low card is at the bottom of a column
        if (are_card_equal(*bottom_card(board, cpp.col), low_card)) {
            stack_push(goal->nextmoves, bottom_card(board, cpp.col));
            stack_push(goal->nextmoves, &board->foundation[suit][board->fdlen[suit]]);
            move(board, &board->freecell[cpp.col], bottom_card(board, cpp.col));
            goal->strat = STRAT_ACCESS_LOW_CARD;
            goal->a = i + 1;
            goal->b = 0;
            return;
        }

        // Card is deeper in a column, consume an empty column with at least 2 cards
        if (goal->b <= 0 && freecol != -1) {
            // Try at sortdepth - 1 as STRAT_BUILD_EMPTY builds at full depth already, super-moves at least 2 cards
            for (depth = board->sortdepth[cpp.col] - 1; depth >= 2; depth--) {
                if (supermove(board, cpp.col, freecol, depth, goal->nextmoves)) {
                    goal->strat = STRAT_ACCESS_LOW_CARD;
                    goal->a = i;
                    goal->b = 1;
                    return;
                }
            }
        }

        // Consume a freecell
        if (goal->b <= 1 && freecell != -1) {
            stack_push(goal->nextmoves, bottom_card(board, cpp.col);
            stack_push(goal->nextmoves, &board->freecell[freecell]);
            move(board, bottom_card(board, cpp.col), &board->freecell[freecell]);
            goal->strat = STRAT_ACCESS_LOW_CARD;
            goal->a = i;
            goal->b = 2;
            return;
        }

        // Last resort, consume a column with only 1 card
        if (goal->b <= 2 && freecol) {
            stack_push(goal->nextmoves, bottom_card(board, cpp.col));
            stack_push(goal->nextmoves, bottom_card(board, freecol) + 1);
            move(board, bottom_card(board, cpp.col), bottom_card(board, freecol) + 1);
            goal->strat = STRAT_ACCESS_LOW_CARD;
            goal->a = i + 1;
            goal->b = 0;
            return;
        }
	}
}

/**
 * Last resort, just move any card
 */
void strat_any_move(Board *board, Goal *goal) {
    
}