int count_freecell(Board *board) {
	int freecell_cnt;

	freecell_cnt = 0;
	for (col = 0; col < 4; col++) {
		freecell_cnt += is_nullcard(board->freecell[col]);
	}

	return freecell_cnt;
}

int count_empty_column(Board *board) {
	int empty_column_cnt;

	empty_column_cnt = 0;
	for (col = 0; col < 8; col++) {
		empty_column_cnt += is_empty(Board, col);
	}

	return empty_column_cnt;
}


/**
 * Determines whether we can move the ``fromcard`` on the ``tocard`` given
 * where (`l` freecell, `f` foundation, `c` columns) the ``tocard`` is.
 */
bool is_move_valid(Card fromcard, Card tocard, char destination) {
	if (fromcard.value == 0)
		return false;

	switch (destination) {
		case 'l':  // freecell
			return tocard.value == 0;
		case 'f':  // foundation
			return (
				fromcard.symbol == tocard.symbol
				&& fromcard.value == tocard.value + 1
			);
		case 'c':  // column
			return (
				tocard.value == 0
				|| (
					fromcard.color != tocard.color
					&& fromcard.value == tocard.value - 1
				)
			);
		default:
			assert(false);
	}
}

/**
 * Determines whether the specified card is nullcard.
 */
bool is_nullcard(Card card) {
	return card.value == nullcard.value;
}

/**
 * Determines wheter the specified column is empty.
 */
bool is_empty(Board *board, int col) {
	return board->colen[col] == 1;
}

/**
 * Determines if the specified column is fully sorted.
 */
bool is_fully_sorted(Board *board, int col) {
	return board->sortdepth[col] == board->colen[col] - 1;
}

/**
 * Determine if the game is won.
 */
bool is_game_won(Board *board) {
	return (
		board->fdlen[0] == 14
		&& board->fdlen[1] == 14
		&& board->fdlen[2] == 14
		&& board->fdlen[3] == 14
	);
}

/**
 * Get the lowest card of the specified column.
 */
card* bottom_card(Board *board, int col) {
	return &(board->columns[col][board->colen[col] - 1]);
}

/**
 * Get the highest sorted card we can move from the specified column.
 */
card* highest_sorted_card(Board *board, int col) {
	return &(board->columns[col][board->colen[col] - board->sortdepth[col]]);
}

/**
 * Compute how many cards we can move at once.
 */
void compute_supermove(Board *board) {
	int cf, cec;

	fc = count_freecell()
	ec = count_empty_column()

	board->supermove = fc() * ec + (1 << ec);
}

/**
 * Compute how many cards are sorted (by the solitaire rule) from the
 * bottom of each column.
 */
void compute_sortdepth(Board *board) {
	int col, depth;
	Card *card;

	for (col = 0; col < 8; col++) {
		for (
			depth = 0, card = bottom_card(board, col);
			!is_nullcard(*card) && is_move_valid(*card, *(card - 1), 'c');
			depth++, card--
		)
		board->sortdepth[col] = depth;
	}
}

/**
 * Compute the "build factor" of each column, the "build factor" indicates
 * how appropriate it is to add cards on the column. It is better (higher
 * bf value) to build on fully sorted columns, it is then better to not
 * build on columns where a lot of low value cards are blocked.
 */
void compute_buildfactor(Board *board) {
	int col, row;
	Card *highcard, *card;

	for (col = 0; col < 8; col++) {
		if (is_empty(board, col)) {
			board->buildfactor[col] = 0;
		} else if (is_fully_sorted(board, col)) {
			board->buildfactor[col] = board->columns[col][1].value * MAXCOLEN + board->colen[col];
		} else {
			board->buildfactor[col] = -board->colen[col];
			highcard = highest_sorted_card(board, col);
			for (card = highcard; !is_nullcard(*card); card--) {
				if (card->value < highcard->value) {
					board->buildfactor[col] -= (highcard->value - card->value);
				} else {
					highcard = card;
				}
			}
		}
	}
}

/**
 * Shuffle a deck of card.
 */
void shuffle(Card *deck, int len) {
	int i, r;
	Card tmp;

	for (i = 0; i < len; i++) {
		r = rand() % len;
		tmp = deck[i];
		deck[i] = deck[r];
		deck[r] = tmp;
	}
}

/**
 * Initializes an empty board.
 */
void board_init(Board *board) {
	int col;
	Card newcard;

	nullcard.value = 0;
	nullcard.symbol = 0;
	nullcard.color = 0;
	nullcard._padding = 0;

	memset(board, 0, sizeof(Board));

	for (col = 0; col < 4; col++) {
		newcard.color = col / 2;
		newcard.symbol = col % 2;
		newcard.value = 0;
		board->foundation[col][0] = newcard;
		board->fdlen[col] = 1;
	}
}

/**
 * Randomly deal a board.
 */
void deal_board(Board *board) {
	int row, col;
	int symbol, color, value;
	Card newcard;
	Card deck[52];

	// Create a deck of card
	for (color = 0; color < 2; color++) {
		for (symbol = 0; symbol < 2; symbol++) {
			for (value = 1; value < 14; value++) {
				newcard.color = color;
				newcard.symbol = symbol;
				newcard.value = value;
				newcard._padding = 0;
				deck[(color * 2 + symbol) * 13 + value - 1] = newcard;
			}
		}
	}
	shuffle(deck, 52);

	// Assign each card to a column
	for (col = 0; col < 8; col++) {
		board->columns[col][0] = nullcard;
	}
	for (row = 1; row < 7; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = deck[(row - 1)* 8 + col];
		}
	}
	for (col = 0; col < 4; col++) {
		board->columns[col][row] = deck[48 + col];
		board->colen[col] = 8;
	}
	for (col = 4; col < 8; col++) {
		board->colen[col] = 7;
	}
	for (row = 8; row < MAXCOLEN; row++) {
		for (col = 0; col < 8; col++) {
			board->columns[col][row] = nullcard;
		}
	}
}

/**
 * Load a board from an ascii text file.
 */
void board_load(Board *board, const char *pathname) {
	int fd, row, col;
	char line[32];
	Card newcard;

	fd = open(pathname, O_RDONLY);
	assert(fd > 2);
	newcard._padding = 0;
	for (row = 0; row < 7; row++) {
		assert(read(fd, line, 32));
		for (col = 0; col < (row == 6 ? 4 : 8); col++) {
			switch (line[col * 4 + 1]) {
				case 'A':
				case '1': newcard.value = 1; break;
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9': newcard.value = line[col * 4 + 1] - '0'; break;
				case '0': newcard.value = 10; break;
				case 'J': newcard.value = 11; break;
				case 'Q': newcard.value = 12; break;
				case 'K': newcard.value = 13; break;
				default: assert(0);
			}
			switch (line[col * 4 + 2]) {
				case 'S': newcard.color = 0; newcard.symbol = 0; break;
				case 'C': newcard.color = 0; newcard.symbol = 1; break;
				case 'H': newcard.color = 1; newcard.symbol = 0; break;
				case 'D': newcard.color = 1; newcard.symbol = 1; break;
				default: assert(0);
			}
			board->columns[col][row] = newcard;
		}
	}
	assert(close(fd) == 0);
	for (col = 0; col < 4; col++) {
		board->colen[col] = 6;
	}
	for (col = 4; col < 8; col++) {
		board->colen[col] = 7;
	}
}

/**
 * Print the board on screen.
 */
void board_show(Board *board) {
	int row, col;

	for (col = 0; col < 4; col++) {
		setcardstr(board->freecell[col]);
		printf("%s ", cardstr);
	}
	printf("|");
	for (col = 0; col < 4; col++) {
		setcardstr(board->foundation[col][board->fdlen[col]-1]);
		printf(" %s", cardstr);
	}
	printf("\n---------------------------------\n");
	for (row = 0; row < 9; row++) {
		for (col = 0; col < 8; col++) {
			setcardstr(board->columns[col][row]);
			printf(" %s", cardstr);
		}
		printf("\n");
	}
}

/**
 * Swap a card and a nullcard on the board. The function is reversible.
 */
void move(Board *board, Card *card1, Card *card2) {
	// Update depth of column and of foundation
	if (card1 >= (Card*) board->columns) {
		board->colen[(card1 - (Card*) board->columns) / MAXCOLEN]--;
	} else if (card1 >= (Card*) board->foundation) {
		board->fdlen[(card1 - (Card*) board->foundation) / MAXFDLEN]--;
	} else {
	}

	if (card2 >= (Card*) board->columns) {
		board->colen[(card2 - (Card*) board->columns) / MAXCOLEN]++;
	} else if (card2 >= (Card*) board->foundation) {
		board->fdlen[(card2 - (Card*) board->foundation) / MAXFDLEN]++;
	} else {
	}

	// Move the card
	*card2 = *card1;
	*card1 = nullcard;
}


bool supermove(Board *board, int fromcol, int tocol, int card_cnt, Stack * nextmoves) {
	/* The bellow example showcase the supermove algorithm using a game
	 * with a total of 2 freecells and 4 columns whose 1 is empty. We want
	 * to move the second (C2) column on the first (C1) column.
	 *
	 *    ###########
	 *    # ___ ___ #########
	 *    #-----------------#
	 *    # Q   8   2   ___ #
	 *    # J   2   6       #
	 *    #     3           #
	 *    #    10           #
	 *    #     9           #
	 *    #     8           #
	 *    #     7           #
	 *    #     6           #
	 *    #     5           #
	 *    #     4           #
	 *    #     3           #
	 *    #     2           #
	 *    #                 #
	 *    ###################
	 *
	 * Move C2 {5, 4, 3, 2} -> C3 | 3 freecells | 2a 2b 24 23 43 b3 a3
	 *  Move C2 {8, 7, 6} -> C4   | 2 freecells | 2a 2b 24 b4 a4
	 *   Move C2 {10, 9} -> C1    | 2 freecells | 2a 21 a1
	 *  Move C4 {8, 7, 6} -> C1   | 2 freecells | 4a 4b 41 b1 a1
	 * Move C3 {5, 4, 3, 2} -> C1 | 3 freecells | 3a 3b 34 31 41 b1 a1
	 */

	int freecell_cnt, empty_column_cnt, col, row;
	Card *fromcard, *tocard;
	Stack *tempmoves;

	tocard = bottom_card(board, tocol);
	fromcard = bottom_card(board, tocol);
	freecell = count_freecell(board) + count_empty_column(board);
	if (is_empty(board, tocol)) empty_column_cnt--;

	// Enough freecells to move the cards right away
	if (card_cnt <= freecell + 1) {
		assert(stack_new(&tempmoves) == CC_OK);

		// Stack as many card in the freecells as needed
		for (col = 0; col < 4 && !is_move_valid(*fromcard, *tocard); col++) {
			if (!is_nullcard(board->freecell[col])) continue
			move(board, fromcard, board->freecell[col]);
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, board->freecell[col]) == CC_OK);
			assert(stack_push(tempmoves, board->freecell[col]) == CC_OK);
			fromcard--;
		}

		// Stack as many card in the empty columns as needed
		for (col = 0; col < 8 && !is_move_valid(*fromcard, tocard); col++) {
			if (!is_empty(board, col) || col == tocol) continue;
			move(board, fromcard, bottom_card(col) + 1);
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, bottom_card(col)) == CC_OK);
			assert(stack_push(tempmoves, bottom_card(col)) == CC_OK);
			fromcard--;
		}

		// Moves the card to the dest column
		assert(is_move_valid(fromcard, tocard));
		tocard++;
		move(board, fromcard, tocard);
		assert(stack_push(nextmoves, fromcard) == CC_OK);
		assert(stack_push(nextmoves, tocard) == CC_OK);
		fromcard--;

		// Unstack the cards from the freecells to the dest column
		while (stack_size(tempmoves)) {
			tocard++;
			assert(stack_pop(tempmoves, &fromcard) == CC_OK);
			move(board, fromcard, tocard);
			assert(stack_push(nextmoves, fromcard) == CC_OK);
			assert(stack_push(nextmoves, tocard) == CC_OK);
		}
		stack_destroy(tempmoves);
		return true;
	}

	// Deep supermove, temporary stack some cards on a non-empty column
	for (col = 0; col < 8; col++) {
		if (is_empty(board, col) || col == tocol) continue;
		tocard = *(bottom_card(board, col));
		for (row = 0; row <= freecell + 1; row++) {
			if (!is_move_valid(*(fromcard - row), tocard)) continue;
			supermove(board, fromcol, col, row, nextmoves);
			if (!supermove(board, fromcol, tocol, card_cnt - row, nextmoves)) {
				return false;
			}
			supermove(board, col, tocol, row, nextmoves);
		}
	}

	// Deep supermove, temporary stack some cards on a non-empty column
	for (col = 0; col < 8; col++) {
		if (!is_empty(board, col) || col == tocol) continue;
		supermove(board, fromcol, col, freecell + 1, nextmoves);
		if (!supermove(board, fromcol, tocol, card_cnt - freecell - 1, nextmoves)) {
			return false;
		}
		supermove(board, col, tocol, freecell + 1, nextmoves);
	}

	// Impossible to supermove
	return false;
}
