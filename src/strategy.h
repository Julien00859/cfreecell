#include "stack.h"

enum strat {
	STRAT_AUTO,
	STRAT_BUILD_NONEMPTY,
};

typedef struct goal {
	Stack * nextmoves;
	enum strat strat;
	int a;
	int b;
	int c;
} Goal;
