#ifndef FREECELL_FREECELL_H
#define FREECELL_FREECELL_H

#include <stdbool.h>
#include <stdint.h>
#include "board.h"
#include "stack.h"
#include "strategy.h"
#include "hashset.h"

typedef struct node {
	struct node *parent;
	Goal *goal;
} Node;

Node* search(Board *board, HashSet *visited);

#endif