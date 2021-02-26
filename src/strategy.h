#ifndef FREECELL_STRATEGY_H
#define FREECELL_STRATEGY_H

#include "stack.h"

enum strat {
	STRAT_AUTO,
	STRAT_BUILD_NONEMPTY,
	STRAT_BUILD_EMPTY,
    STRAT_ACCESS_LOW_CARD,
};

typedef struct goal {
	Stack * nextmoves;
	enum strat strat;
	int a;
	int b;
	int c;
} Goal;


bool respect_rule_of_two(Board *board, Card fromcard);
void strat_auto_move(Board *board, Goal *goal);
int comp_buildfactor(const void *p1, const void *p2, const void *arg);
void strat_build_nonempty(Board *board, Goal *goal);
void strat_build_empty(Board *board, Goal *goal);
void strat_access_low_card(Board *board, Goal *goal);

#endif