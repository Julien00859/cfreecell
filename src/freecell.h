#include <stdbool.h>
#include <stdint.h>
#include "stack.h"

typedef struct node {
	struct node *parent;
	Card *lastfromcard;
	Card *lasttocard;
} Node;

void setcardstr(Card card);
void listmoves(Board *board, Stack * nextmoves, Node *node);
int play_cost(Board *board, Card *tocard);
void play(Board *board, Card *card1, Card *card2);
Node* search(Board * board, Node * currentnode);
