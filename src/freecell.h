#include <stdbool.h>
#include <stdint.h>
#include "stack.h"
#include "hashset.h"

#define MAXFDLEN 14
#define MAXCOLEN 21

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

void shuffle(Card *deck, int len);
void board_init(Board *board);
void board_show(Board *board);
void card_str(Card card, char *str);
bool validate_move(Card fromcard, Card tocard, char destination);
void listmoves(Board *board, Stack * nextmoves);
void play(Board *board, Card *card1, Card *card2);
bool explore(Board *board, HashSet *visited_boards);
size_t hash(const void *key, int l, uint32_t seed);
