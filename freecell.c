#include <stdlib.h>
#include <stdio.h>

#define SPADE 0
#define CLUB 16
#define HEART 32
#define DIAMOND 48


static char fcslots[4];
static char fdslots[4];
static char columns[8][20];

char blankcard =  0b01111111;
char colormask =  0b00100000;  // 0 black, 1 red
char symbolmask = 0b00110000;  // 0 spade, 1 club, 2 heart, 3 diamond
char valuemask =  0b00001111;  // 1 ace, 2 two, ..., 10 ten, ..., 13 king

void board_clear() {
	int row, col;

	for (col = 0; col < 8; col++) {
		fcslots[col] = blankcard;
	}

	for (col = 0; col < 8; col++) {
		fdslots[col] = blankcard;
	}

	for (row = 0; row < 20; row++) {
		for (col = 0; col < 8; col++) {
			columns[row][col] = blankcard;
		}
	}
}

void board_init() {
	char fulldeck[52];
	char value, symbol;
	int row, col;

	for (symbol = 0; symbol < 4; symbol++) {
		for (value = 0; value < 14; value++) {
			fulldeck[symbol * 13 + value] = symbol * 16 + value + 1;
		}
	}

	for (row = 0; row < 6; row++) {
		for (col = 0; col < 8; col++) {
			columns[row][col] = fulldeck[row * 8 + col];
		}
	}
	columns[6][0] = fulldeck[48];
	columns[6][1] = fulldeck[49];
	columns[6][2] = fulldeck[50];
	columns[6][3] = fulldeck[51];
}

void card_str(char card, char * str) {
	if (card == blankcard) {
		str[0] = str[1] = str[2] = ' ';
		return;
	}
	str[0] = ' ';
	switch (card & valuemask) {
		case 10: str[0] = '1'; str[1] = '0'; break;
		case 11: str[1] = 'J'; break;
		case 12: str[1] = 'Q'; break;
		case 13: str[1] = 'K'; break;
		default: str[1] = '0' + (card & valuemask);
	}
	switch (card & symbolmask) {
		case SPADE: str[2] = 'S'; break;
		case CLUB: str[2] = 'C'; break;
		case HEART: str[2] = 'h'; break;
		case DIAMOND: str[2] = 'd'; break;
	}
}

void board_show() {
	int row, col;
	char str[4];
	str[3] = 0;

	for (col = 0; col < 4; col++) {
		card_str(fcslots[col], str);
		printf("%s ", str);
	}
	printf("|");
	for (col = 0; col < 4; col++) {
		card_str(fdslots[col], str);
		printf(" %s", str);
	}
	printf("\n---------------------------------\n");
	for (row = 0; row < 20; row++) {
		for (col = 0; col < 8; col++) {
			card_str(columns[row][col], str);
			printf(" %s", str);
		}
		printf("\n");
	}
}



int main(void) {
	board_clear();
	board_init();
	board_show();
	return 0;
}
