#ifndef FREECELL_BOARD_H
#define FREECELL_BOARD_H

#include <stdbool.h>
#include <stdint.h>
#include "stack.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define KING 13
#define MAXFDLEN 14
#define MAXCSLEN 20

/**
 * Immutable card, there are 52 + the nullcard
 */
typedef struct card {
	uint8_t _padding:2;
	uint8_t color:1;
	uint8_t suit:1;
	uint8_t rank:4;
} Card;

/**
 * Mutable board as we cannot afford to keep each board in memory
 */
typedef struct board {
	Card freecell[4];
	Card foundation[4][MAXFDLEN];
	Card cascade[8][MAXCSLEN];
	uint8_t fdlen[4];
	uint8_t cslen[8];

	// Properties, must be recalculated after each move
	uint8_t sortdepth[8];
	int buildfactor[8];
} Board;

typedef struct cardpospair {
	unsigned int col:3;
	unsigned int row:5;
} CardPosPair;

static Card nullcard;

int count_freecell(Board *board);
int count_empty_column(Board *board);

bool is_move_valid(Card fromcard, Card tocard, char destination);
bool is_nullcard(Card card);
bool is_empty(Board *board, int col);
bool is_fully_sorted(Board *board, int col);
bool is_game_won(Board *board);
bool are_card_equal(Card c1, Card c2);

Card* bottom_card(Board *board, int col);
Card* highest_sorted_card(Board *board, int col);
CardPosPair search_card(Board *board, Card searched_card);

void compute_sortdepth(Board *board);
void compute_sortdepth_col(Board *board, int col);
void compute_buildfactor(Board *board);

void shuffle(Card *deck, int len);
void setcardstr(Card card, char *cardstr);
void setmovestr(Board *board, Card *fromcard, Card *tocard, char *movestr);
void board_init(Board *board);
void board_deal(Board *board);
void board_load(Board *board, const char *pathname);
void board_show(Board *board);

void move(Board *board, Card *card1, Card *card2);
void humanmove(Board *board, int fromcol, int tocol);
int supermove_depth(Board *board, int fromcol, int tocol);
bool supermove(Board *board, int fromcol, int tocol, int card_cnt, Stack * nextmoves);
bool deepsupermove(Board *board, int fromcol, int tempcol, int tocol, int total_card_cnt, int card_cnt, Stack *nextmoves);
bool superaccess(Board *board, CardPosPair cpp, Stack * nextmoves, bool use_empty);

#endif
