#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>
#include "board.h"
#include "freecell.h"
#include "stack.h"
#include "strategy.h"


// Stats
unsigned long long play_cnt;
unsigned long long nodes_cnt;
unsigned long int start_time;

Node* search(Board *board, Node *currentnode) {
	return NULL;
}

int main(int argc, char *argv[]) {
	Board board;
	Goal goal;
	Node rootnode;
	Node *leaf;
	Stack *nextmoves;

	board_init(&board);

	if (argc == 1) {
		int seed;
		assert(getrandom(&seed, sizeof(seed), 0) != -1);
		srand(seed);
		board_deal(&board);
		printf("Seed: %d\n\n", seed);
	} else if (argc == 2) {
		srand(strtol(argv[1], NULL, 10));
		board_deal(&board);
		printf("Seed: %s\n\n", argv[1]);
	} else if (argc == 3) {
		board_load(&board, argv[2]);
		printf("File: %s\n\n", argv[2]);
	}

	board_show(&board);

    humanmove(&board, 8, 0xa);
    stack_new(&goal.nextmoves);
    strat_auto_move(&board, &goal);
    board_show(&board);

    humanmove(&board, 7, 8);
    humanmove(&board, 1, 8);
    humanmove(&board, 1, 0xb);
    humanmove(&board, 1, 0xc);
    humanmove(&board, 1, 0xd);
    humanmove(&board, 1, 2);
    humanmove(&board, 1, 8);
    humanmove(&board, 1, 0);
    humanmove(&board, 7, 2);
    humanmove(&board, 7, 0);
    humanmove(&board, 6, 0);
    humanmove(&board, 7, 0);
    humanmove(&board, 7, 6);
    humanmove(&board, 7, 5);
    humanmove(&board, 0xc, 7);
	board_show(&board);

    compute_sortdepth(&board);
    compute_buildfactor(&board);
	goal.a = 7;
	goal.b = 0;
	goal.c = 0;
    strat_build_nonempty(&board, &goal);
    board_show(&board);
    compute_sortdepth(&board);
    compute_buildfactor(&board);
    goal.a = 7;
    goal.b = 0;
    goal.c = 0;
    strat_build_nonempty(&board, &goal);
    humanmove(&board, 8, 0xb);
    compute_sortdepth(&board);
    compute_buildfactor(&board);
    goal.a = 7;
    goal.b = 0;
    goal.c = 0;
    strat_build_empty(&board, &goal);
    compute_sortdepth(&board);
    compute_buildfactor(&board);
    goal.a = 7;
    goal.b = 0;
    goal.c = 0;
    strat_build_nonempty(&board, &goal);
    board_show(&board);
    printf("%d\n", stack_size(goal.nextmoves));

	return 0;
}
