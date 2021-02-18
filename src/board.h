#include <stdbool.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MAXFDLEN 14
#define MAXCOLEN 22

/**
 * Immuatable card, there are 52 + the nullcard
 */
typedef struct card {
	unsigned int _padding:2;
	unsigned int color:1;
	unsigned int symbol:1;
	unsigned int value:4;
} Card;

/**
 * Mutable board as we cannot afford to keep each board in memory
 */
typedef struct board {
	Card freecell[4];
	Card foundation[4][MAXFDLEN];
	Card columns[8][MAXCOLEN];
	uint8_t fdlen[4];
	uint8_t colen[8];

	// Properties, must be recalculated after each move
	int supermove;
	uint8_t sortdepth[8];
	int buildfactor[8];
} Board;

int count_freecell(Board *board);

bool is_move_valid(Card fromcard, Card tocard, char destination);
bool is_nullcard(Card card);
bool is_empty(Board *board, int col);
bool is_fully_sorted(Board *board, int col);
bool is_game_won(Board *board);

card* bottom_card(Board *board, int col);
card* highest_sorted_card(Board *board, int col);

void compute_supermove(Board *board);
void compute_sortdepth(Board *board);
void compute_buildfactor(Board *board);

void shuffle(Card *deck, int len);
void board_init(Board *board);
void board_deal(Board *board);
void board_load(Board *board, const char *pathname);
void board_show(Board *board);

void move(Board *board, Card *card1, Card *card2);
bool supermove(Board *board, int fromcol, int tocol, Stack * nextmoves);
