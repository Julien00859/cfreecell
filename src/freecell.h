#include <stdbool.h>
#include <stdint.h>
#include "stack.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MAXFDLEN 14
#define MAXCOLEN 22

typedef struct card {
	unsigned int _padding:2;
	unsigned int color:1;
	unsigned int symbol:1;
	unsigned int value:4;
} Card;

typedef struct board {
	Card freecell[4];
	Card foundation[4][MAXFDLEN];
	Card columns[8][MAXCOLEN];
	uint8_t fdlen[4];
	uint8_t colen[8];
} Board;

typedef struct node {
	struct node *parent;
	Card *lastfromcard;
	Card *lasttocard;
} Node;

void setcardstr(Card card);
bool validate_move(Card fromcard, Card tocard, char destination);
bool check_immediate(Board *board, Card *fromcard);
void shuffle(Card *deck, int len);
void board_init(Board *board);
void board_deal(Board *board);
void board_load(Board *board, const char *pathname);
void board_show(Board *board);
bool isgameover(Board *board);
void listmoves(Board *board, Stack * nextmoves, Node *node);
int play_cost(Board *board, Card *tocard);
void play(Board *board, Card *card1, Card *card2);
Node* search(Board * board, Node * currentnode);
