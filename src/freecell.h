#include <stdbool.h>
#include <stdint.h>
#include "board.h"
#include "stack.h"

typedef struct node {
	struct node *parent;
	Card *lastfromcard;
	Card *lasttocard;
} Node;

void listmoves(Board *board, Stack * nextmoves, Node *node);
int play_cost(Board *board, Card *tocard);
Node* search(Board * board, Node * currentnode);
