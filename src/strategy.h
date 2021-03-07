#ifndef FREECELL_STRATEGY_H
#define FREECELL_STRATEGY_H

#include "stack.h"

enum strat {
    STRAT_NULL,   // When we are still searching
    STRAT_RULE_OF_TWO,
    STRAT_BUILD_DOWN,
    STRAT_BUILD_EMPTY,
    STRAT_ACCESS_LOW_CARD,
    STRAT_ACCESS_BUILD_CARD,
    STRAT_ACCESS_EMPTY,
    STRAT_ANY_MOVE_CASCADE,
    STRAT_ANY_MOVE_FOUNDATION,
    STRAT_ANY_MOVE_FREECELL,  // Last resort
};

typedef struct goal {
	Stack * nextmoves;
	enum strat strat;
	int a;
	int b;
	int c;
} Goal;


bool respect_rule_of_two(Board *board, Card fromcard);
int comp_buildfactor(const void *p1, const void *p2, const void *arg);
int comp_highest_sorted_card(const void *p1, const void *p2, const void *arg);
int comp_fdlen(const void *p1, const void *p2, const void *arg);


void strat_rule_of_two(Board *board, Goal *goal);
void strat_delete_any(Board *board, Goal *goal);
void strat_build_down(Board *board, Goal *goal);
void strat_build_empty(Board *board, Goal *goal);
void strat_access_low_card(Board *board, Goal *goal);
void strat_access_build_card(Board *board, Goal *goal);
void strat_access_empty(Board *board, Goal *goal);
void strat_any_move_cascade(Board *board, Goal *goal);
void strat_any_move_foundation(Board *board, Goal *goal);
void strat_any_move_freecell(Board *board, Goal *goal);

#endif
