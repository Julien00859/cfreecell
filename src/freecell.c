#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "board.h"
#include "freecell.h"
#include "stack.h"
#include "strategy.h"
#include "xxhash.h"
#include "hashset.h"

// Hashset hash function but the object is hashed already
size_t hash_hashkey(const void *key, int l, uint32_t seed) {
    return (size_t)key;
}

// Hashset comparison function but the object is hashed already
int comp_hashkey(const void *k1, const void *k2) {
    XXH64_hash_t h1 = (XXH64_hash_t)k1;
    XXH64_hash_t h2 = (XXH64_hash_t)k2;

    if (h1 < h2) return -1;
    if (h1 > h2) return 1;
    return 0;
}


Node* search(Board *board, HashSet *visited) {
    int strat;
    Stack *nextmoves;
    Goal *goal;
    Node *old_node, *node;
    Card *fromcard, *tocard;
    XXH64_hash_t board_hash;

    // There are many strategy sorted in this array by preference, each
    // index also map to the internal value of the "strat" enum.
    void (*strategies[])(Board *, Goal *) = {
            NULL,
            strat_rule_of_two,
            strat_build_down,
            strat_build_empty,
            strat_access_low_card,
            strat_access_build_card,
            strat_access_empty,
            strat_any_move_cascade,
            strat_any_move_foundation,
            strat_any_move_freecell,
    };

    // Each strategy uses special "initializers" so it is possible to
    // fast-forward their internal loop when we backtrack. This array
    // contains the starting values, e.g. "0" in "for (i = 0; i < 10; i++)"
    int goal_inits[10][2] = {
            {0, 0},  // STRAT_NULL
            {0, 0},  // STRAT_RULE_OF_TWO
            {7, -4},  // STRAT_BUILD_DOWN
            {11, 0},  // STRAT_BUILD_EMPTY
            {0, 0},  // STRAT_ACCESS_LOW_CARD
            {7, 0},  // STRAT_ACCESS_BUILD_CARD
            {7, 0},  // STRAT_ACCESS_EMPTY
            {0, -4},  // STRAT_ANY_MOVE_CASCADE
            {0, 0},  // STRAT_ANY_MOVE_FOUNDATION
            {0, 0},  // STRAT_ANY_MOVE_FREECELL
    };

    // The root node has no parent
    node = NULL;

    RECURSION:;
    while (!is_game_won(board)) {

        // Create a new node, a new goal and a new move stack. Link them
        // together and to new node to the old node.
        old_node = node;
        node = (Node*)malloc(sizeof(Node));
        assert(node != NULL);
        goal = (Goal*)malloc(sizeof(Goal));
        assert(goal != NULL);
        assert(stack_new(&nextmoves) == CC_OK);
        goal->nextmoves = nextmoves;
        goal->strat = STRAT_NULL;
        node->parent = old_node;
        node->goal = goal;

        // Recompute the various board properties
        compute_sortdepth(board);
        compute_buildfactor(board);

        // Test all strategies on un-visited boards
        board_hash = XXH3_64bits(board, (size_t*)board->fdlen - (size_t*)board);
        if (!hashset_contains(visited, (void*)board_hash)) {
            hashset_add(visited, (void*)board_hash);

            for (strat = 1; strat < 10; strat++) {
                goal->a = goal_inits[strat][0];
                goal->b = goal_inits[strat][1];
                RECURSION_RETURN:;
                strategies[strat](board, goal);
                if (goal->strat != STRAT_NULL) {
                    // MATCH! Re-search on the modified board. If the
                    // sub-search fails, we want to resume the current
                    // strategy, we go back to RECURSION_RETURN.
                    goto RECURSION;
                }
            }
        }

        // Board fully visited, no strategy worked, restore the previous node state
        assert(!stack_size(nextmoves));
        stack_destroy(nextmoves);
        free(node->goal);
        old_node = node;
        node = node->parent;
        free(old_node);
        goal = node->goal;
        nextmoves = goal->nextmoves;

        // Restore the board too
        while (stack_size(nextmoves)) {
            assert(stack_pop(nextmoves, (void**)&fromcard) == CC_OK);
            assert(stack_pop(nextmoves, (void**)&tocard) == CC_OK);
            move(board, fromcard, tocard);
        }

        // Continue searching using the previous (now current) node next's strategy
        if (node != NULL) {
            strat = (int)goal->strat;
            goto RECURSION_RETURN;
        }

        // The game is impossible, we backtracked above the root node
        return NULL;
    }

    // The game is solved !
    return node;
}

int main(int argc, char *argv[]) {
	Board board;
	Node *leaf, *old_leaf;
	Card *fromcard;
	Card *tocard;
    HashSet *visited;
    HashSetConf conf;

    // Initiate an empty board
	board_init(&board);

	if (argc == 1) {
	    printf("usage: %s <seed>\n       %s _ <path>\n", argv[0], argv[0]);
	    return 1;
	} else if (argc == 2) {
		srand(strtol(argv[1], NULL, 10));
		board_deal(&board);
		printf("Seed: %s\n\n", argv[1]);
	} else if (argc == 3) {
		board_load(&board, argv[2]);
		printf("File: %s\n\n", argv[2]);
	}

    /* Initiate a "have this board been visited before ?" hashset.
     * Because the board itself is mutable, it is unsafe to use it
     * as hash key. We instead manually hash the board to "freeze"
     * it. The hash key is a hash already, the above hash_hashkey
     * and comp_hashkey just passthrough the manually computed hash
     * and compare the hash themselves. */
    hashset_conf_init(&conf);
    conf.hash = hash_hashkey;
    conf.key_compare = comp_hashkey;
    conf.initial_capacity = 1 << 20;  // 1M
    hashset_new_conf(&conf, &visited);

	// Show the initial board than search for a solution
	board_show(&board);
	leaf = search(&board, visited);

	if (is_game_won(&board)) {
	    printf("Game solved! Most recent move first.\n");
        while (leaf) {
            switch (leaf->goal->strat) {
                case STRAT_RULE_OF_TWO: printf("Rule of two\n"); break;
                case STRAT_BUILD_DOWN: printf("Build down\n"); break;
                case STRAT_BUILD_EMPTY: printf("Build empty\n"); break;
                case STRAT_ACCESS_LOW_CARD: printf("Access low card\n"); break;
                case STRAT_ACCESS_BUILD_CARD: printf("Access build card\n"); break;
                case STRAT_ACCESS_EMPTY: printf("Empty column\n"); break;
                case STRAT_ANY_MOVE_CASCADE: printf("Move any card(s) on the cascades\n"); break;
                case STRAT_ANY_MOVE_FOUNDATION: printf("Move any card to the foundation\n"); break;
                case STRAT_ANY_MOVE_FREECELL: printf("Move any card(s) to the freecells\n"); break;
                default: assert(0);
            }
            while (stack_size(leaf->goal->nextmoves)) {
                stack_pop(leaf->goal->nextmoves, (void**)&tocard);
                stack_pop(leaf->goal->nextmoves, (void**)&fromcard);
                setcardstr(*fromcard);
                setmovestr(&board, fromcard, tocard);
                printf("%s (%s)\n", movestr, cardstr);
            }
            stack_destroy(leaf->goal->nextmoves);
            free(leaf->goal);
            old_leaf = leaf;
            leaf = leaf->parent;
            free(old_leaf);
        }
    } else {
	    printf("Game is unsolvable.");
	}

	hashset_remove_all(visited);
	hashset_destroy(visited);

	return is_game_won(&board) ? 0 : 1;
}
