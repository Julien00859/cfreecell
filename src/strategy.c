#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "board.h"
#include "stack.h"
#include "strategy.h"
#include "isort.h"

/**
 * The "Rule of Two" is a freecell property that indicates a card can be
 * automatically moved to the foundation.
 *
 * TL;DR, a card can be moved to the foundation if its value is lower or
 * equal to the foundation's card of the opposite color least value + 2.
 *
 * The "Rule of Two" is based on the following property : it is impossible
 * to move a card bellow the lowest card (value wise) that is not on the
 * foundation. By the rule of the freecell game, it is forbidden to move a
 * card from the foundation. Because there are no card still "in game" that
 * could be moved bellow the considered card, it can be safely move to the
 * foundation. This is the "least foundation value + 1" card property.
 *
 * By the "least foundation value + 1" property, we know that those cards
 * will be immediately moved to the foundation. They will not be moved to
 * another column on "least foundation value + 2" cards. It means that no
 * card will be moved to "least foundation value + 2" cards either thus it
 * is safe to move them to the foundation too.
 *
 * On the other hand, the "least foundation value + 3" cannot be
 * automatically moved. This is not always safe as we cannot guarantee
 * without more knowledge that the card will not be needed to move a "least
 * foundation value + 2" card blocking the same symbol "least foundation
 * value + 1" card.
 */
bool respect_rule_of_two(Board *board, Card fromcard) {
	return fromcard.value <= MIN(
		board->fdlen[(1 - fromcard.color) * 2 + 0],
		board->fdlen[(1 - fromcard.color) * 2 + 1]
	) + 1;  // +2 but there is a nullcard on top
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


int comp_highest_sorted_card(const void *p1, const void *p2, const void *arg) {
    int col1, col2, card_value1, card_value2;
    Board *board;

    board = (Board*) arg;
    col1 = *((int*)p1);
    col2 = *((int*)p2);
    card_value1 = col1 >= 0 ? highest_sorted_card(board, col1)->value : board->freecell[col1 + 4].value;
    card_value2 = col2 >= 0 ? highest_sorted_card(board, col2)->value : board->freecell[col2 + 4].value;

    if (card_value1 > card_value2)
        return 1;
    else if (card_value1 < card_value2)
        return -1;
    return 0;
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

int comp_collen(const void *p1, const void *p2, const void *arg) {
    int col1, col2;
    Board *board;

    col1 = *((int*)p1);
    col2 = *((int*)p2);
    board = (Board*) arg;

    if (board->colen[col1] > board->colen[col2])
        return 1;
    else if (board->colen[col1] > board->colen[col2])
        return -1;
    return 0;
}


/**
 * It is always possible to move the cards with the least value in the
 * cascades and freecells to the foundation.
 */
void strat_rule_of_two(Board *board, Goal *goal) {
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
		fromcard = bottom_card(board, fromcol);
		symbol = fromcard->color * 2 + fromcard->symbol;
		tocard = &(board->foundation[symbol][board->fdlen[symbol] - 1]);
		if (is_move_valid(*fromcard, *tocard, 'f') && respect_rule_of_two(board, *fromcard)) {
			assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
			assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
            move(board, fromcard, tocard + 1);
		}
	}

	if (stack_size(goal->nextmoves)) {
		goal->strat = STRAT_RULE_OF_TWO;
	}
}


/**
 * Use the build-factor (a value computing "how well" a column is sorted) to
 * move cards from columns with a low factor to columns with a high one.
 */
void strat_build_down(Board *board, Goal *goal) {
	int i, j, tocol, fromcol, depth;
	Card *tocard, *fromcard;
	int columns[] = {0, 1, 2, 3, 4, 5, 6, 7};

	isort_r(columns, 8, sizeof(int), comp_buildfactor, board);

	for (i = goal->a; i >= 0; i--) {  // i = 7, highest build factor
		tocol = columns[i];
		if (is_empty(board, tocol)) continue;
		tocard = bottom_card(board, tocol);

		// From freecell to column
		for (fromcol = goal->b; fromcol < 0; fromcol++) {  // fromcol = -4
			fromcard = &(board->freecell[fromcol + 4]);
			if (is_move_valid(*fromcard, *tocard, 'c')) {
				assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
				assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
				move(board, fromcard, tocard + 1);
				goal->strat = STRAT_BUILD_DOWN;
				goal->a = i;
				goal->b = fromcol + 1;
				return;
			}
		}

		// From column to column
		for (j = MAX(0, goal->b); j < i; j++) {  // j = 0, lowest build factor
			fromcol = columns[j];
			if (is_empty(board, fromcol)) continue;

			depth = supermove_depth(board, fromcol, tocol);
			if (!depth) continue;

			if (supermove(board, fromcol, tocol, depth, goal->nextmoves)) {
				goal->strat = STRAT_BUILD_DOWN;
				goal->a = i;
				goal->b = j + 1;
				return;
			}
		}
	}
}

/**
 * Start empty columns high (king first) skipping fully-sorted columns and
 * partially-sorted columns when it is impossible to move all the sorted cards.
 */
void strat_build_empty(Board *board, Goal *goal) {
	int fromcol, tocol, columns_length, i;
	int columns[12];  // Maximum 8 columns + 4 freecells, columns are indexed 0->7, freecells are indexed -4->-1

	// Find an empty column
	for (tocol = 0; tocol < 8 && !is_empty(board, tocol); tocol++);
	if (tocol == 8) return;

	// Complete columns[] with non-empty, non-fully-sorted column indexes
	// sort it by highest sorted card
    columns_length = 0;
    for (fromcol = -4; fromcol < 0; fromcol++) {  // hack, freecell indexes are negatives
        if (is_nullcard(board->freecell[fromcol + 4])) continue;
        columns[columns_length++] = fromcol;
    }
    for (; fromcol < 8; fromcol++) {
        if (is_empty(board, fromcol)) continue;
        if (is_fully_sorted(board, fromcol)) continue;
        columns[columns_length++] = fromcol;
    }
    if (!columns_length) return;
    isort_r(columns, columns_length, sizeof(int), comp_highest_sorted_card, board);

    for (i = MIN(goal->a, columns_length - 1); i >= 0; i--) {  // i = 11
        fromcol = columns[i];

        // From freecell to empty column
        if (fromcol < 0) {
            stack_push(goal->nextmoves, &board->freecell[fromcol + 4]);
            stack_push(goal->nextmoves, bottom_card(board, tocol) + 1);
            move(board, &board->freecell[fromcol + 4], bottom_card(board, tocol) + 1);
            goal->strat = STRAT_BUILD_EMPTY;
            goal->a = i - 1;
            return;
        }

        // From column to empty column (only full move)
        if (supermove(board, fromcol, tocol, board->sortdepth[fromcol], goal->nextmoves)) {
            goal->strat = STRAT_BUILD_EMPTY;
            goal->a = i - 1;
            return;
        }
    }
}


/**
 * Find a card that is low and destruct the column to access it
 */
void strat_access_low_card(Board *board, Goal *goal) {
	int i, suit;
	int suits[4] = {0, 1, 2, 3};
	Card low_card, *card;
	CardPosPair cpp;

	isort_r(suits, 4, sizeof(int), comp_fdlen, board);

	for (i = goal->a; i < 4; i++) {  // i = 0
	    suit = suits[i];
        low_card.color = suit & 0b10;
        low_card.symbol = suit & 0b01;
        low_card.value = board->fdlen[suit];

        if (low_card.value == KING + 1)
            continue;

        cpp = search_card(board, low_card);

        // Low card found on freecell, from freecell to foundation
        if (cpp.row == MAXCOLEN) {  // Awful hack
            stack_push(goal->nextmoves, &board->freecell[cpp.col]);
            stack_push(goal->nextmoves, &board->foundation[suit][board->fdlen[suit]]);
            move(board, &board->freecell[cpp.col], &board->foundation[suit][board->fdlen[suit]]);
            goal->strat = STRAT_ACCESS_LOW_CARD;
            goal->a = i + 1;
            return;
        }

        // Spread cards around until ours is accessible
        if (!superaccess(board, cpp, goal->nextmoves, true)) continue;

        // Low card found at bottom of column, from column to foundation
        stack_push(goal->nextmoves, bottom_card(board, cpp.col));
        stack_push(goal->nextmoves, &board->foundation[suit][board->fdlen[suit]]);
        move(board, bottom_card(board, cpp.col), &board->foundation[suit][board->fdlen[suit]]);
        goal->strat = STRAT_ACCESS_LOW_CARD;
        goal->a = i + 1;
        return;
	}
}

void strat_access_build_card(Board *board, Goal *goal) {
    int i, s, suit, tocol, columns_length;
    CardPosPair cpp;
    Card *tocard, build_card;
    int columns[8];

    // Make a list sorted by build-factor of non-empty fully sorted columns
    columns_length = 0;
    for (tocol = goal->a; tocol < 8; tocol++) {
        if (is_empty(board, tocol)) continue;
        if (!is_fully_sorted(board, tocol)) continue;
        columns[columns_length++] = tocol;
    }
    if (!columns_length) return;
    isort_r(columns, columns_length, sizeof(int), comp_buildfactor, board);

    // For each column...
    for (i = MIN(goal->a, columns_length - 1); i >= 0; i--) {  // i = 7
        tocol = columns[i];
        tocard = bottom_card(board, tocol);

        // ...search for a card that can be moved at the bottom
        build_card.value = tocard->value - 1;
        build_card.color = 1 - tocard->color;
        for (s = goal->b; s < 2; s++) {  // // s = 0
            build_card.symbol = s;
            suit = build_card.color * 2 + build_card.symbol;
            if (build_card.value <= board->fdlen[suit]) continue;  // card is on the foundation already
            cpp = search_card(board, build_card);

            // If the card is movable right away then STRAT_BUILD_DOWN already did search it
            if (cpp.row == MAXCOLEN) continue;
            if (cpp.row >= board->colen[cpp.col] - board->sortdepth[cpp.col]) continue;

            // Spread cards around until ours is accessible
            if (!superaccess(board, cpp, goal->nextmoves, true)) continue;

            // There is a chance the card of the other symbol (same color)
            // was under the build_card, in such case there is a change it
            // was moved under tocard already, in such case the bellow
            // conditional is false but the strategy is still validated
            if (bottom_card(board, tocol) == tocard) {
                // Build using the now accessible card
                stack_push(goal->nextmoves, bottom_card(board, cpp.col));
                stack_push(goal->nextmoves, tocard + 1);
                move(board, bottom_card(board, cpp.col), tocard + 1);
            }
            goal->strat = STRAT_ACCESS_BUILD_CARD;
            goal->a = i;
            goal->b = s + 1;
        }
    }
}


void strat_access_empty(Board *board, Goal *goal) {
    int i;
    int columns[] = {0, 1, 2, 3, 4, 5, 6, 7};
    CardPosPair cpp;

    isort_r(columns, 8, sizeof(int), comp_collen, board);

    for (i = goal->a; i >= 0; i--) {  // i = 7
        cpp.row = 0;
        cpp.col = columns[i];

        if (is_fully_sorted(board, cpp.col) && board->colen[cpp.col] > 4) continue;
        if (!superaccess(board, cpp, goal->nextmoves, false)) continue;

        goal->strat = STRAT_ACCESS_EMPTY;
        goal->a = i - 1;
    }
}


void strat_any_move_cascade(Board *board, Goal *goal) {
    int fromcol, tocol, depth;
    Card *fromcard, *tocard;

    // To cascade...
    for (tocol = goal->a; tocol < 8; tocol++) {  // tocol = 0;
        tocard = bottom_card(board, tocol);

        // ... from freecell
        fromcol = goal->b;
        for (; fromcol < 0; fromcol++) {  // fromcol = -4;
            fromcard = &board->freecell[fromcol + 4];
            if (!is_move_valid(*fromcard, *tocard, 'c')) continue;
            assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
            assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
            move(board, fromcard, tocard + 1);
            goal->strat = STRAT_ANY_MOVE_CASCADE;
            goal->a = tocol;
            goal->b = fromcol + 1;
            return;
        }

        // ... from another cascade
        for (; fromcol < 8; fromcol++) {
            if (fromcol == tocol) continue;
            if (is_empty(board, fromcol)) continue;

            depth = supermove_depth(board, fromcol, tocol);
            if (!depth) continue;

            if (!supermove(board, fromcol, tocol, depth, goal->nextmoves)) continue;
            goal->strat = STRAT_ANY_MOVE_CASCADE;
            goal->a = tocol;
            goal->b = fromcol + 1;
            return;
        }
    }
}

void strat_any_move_foundation(Board *board, Goal *goal) {
    int fromcol, suit;
    Card *fromcard, *tocard;

    for (fromcol = goal->a; fromcol < 8; fromcol++) {  // fromcol = 0
        fromcard = fromcol < 0 ? &board->freecell[fromcol + 4] : bottom_card(board, fromcol);
        suit = fromcard->color * 2 + fromcard->symbol;
        tocard = &board->foundation[suit][board->fdlen[suit] - 1];
        if (!is_move_valid(*fromcard, *tocard, 'f')) continue;

        assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
        assert(stack_push(goal->nextmoves, tocard + 1) == CC_OK);
        move(board, fromcard, tocard + 1);
        goal->strat = STRAT_ANY_MOVE_FOUNDATION;
        goal->a = fromcol + 1;
        return;
    }
}

void strat_any_move_freecell(Board *board, Goal *goal) {
    int freecell_cnt, fromcol, i, depth;
    Card *freecells[12];
    Card *fromcard, *tocard;

    // Stack freecells and empty columns (freecells on top)
    freecell_cnt=0;
    for (fromcol = 0; fromcol < 8; fromcol++)
        if (is_empty(board, fromcol))
            freecells[freecell_cnt++] = &board->columns[fromcol][0];
    for (fromcol = 0; fromcol < 4; fromcol++)
        if (is_nullcard(board->freecell[fromcol]))
            freecells[freecell_cnt++] = &board->freecell[fromcol];
    if (!freecell_cnt) return;

    // Find a column from which we can move all sorted cards to freecells
    for (fromcol = goal->a; fromcol < 8; fromcol++) {  // fromcol = 0
        if (is_empty(board, fromcol)) continue;
        depth = board->sortdepth[fromcol];
        if (depth > freecell_cnt) continue;

        for (fromcard = bottom_card(board, fromcol), i = 0; i < depth; fromcard--, i++) {
            tocard = freecells[freecell_cnt--];
            assert(stack_push(goal->nextmoves, fromcard) == CC_OK);
            assert(stack_push(goal->nextmoves, tocard) == CC_OK);
            move(board, fromcard, tocard);
        }
        goal->strat = STRAT_ANY_MOVE_FREECELL;
        goal->a = fromcol + 1;
        return;
    }
}