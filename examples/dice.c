#include <stdio.h>
#define JAP_DICE_IMP 1
#include "../jap_dice.h"

int parsetest(char* dice) {
	jap_diceroll roll;
	int res;

	printf("Parsing %s...\n", dice);
	res = jdice_parse(dice, &roll);
	if (res) {
		printf("Couldn't parse %s\n", dice);
		return 1;
	}
	printf("Parse result: n=%i, x=%i, type=%i\n", roll.n, roll.x, roll.type);
	printf("Rolled: %i\n", jdice_roll(&roll));
	return 0;
}

int main() {
	srand(6969);
	parsetest("2d10");
	parsetest("3D");
	parsetest("4dF");
	parsetest("+2d20");
	parsetest("-3d10");

	printf("I'm going to roll 100d6.\n");
	for (int i = 0; i < 100; i++) {
		if (i!=0) printf(", ");
		printf("%i", jdice_ndx(1, 6));
	}
	putchar('\n');

	printf("Now let's try 10dF.\n");
	for (int i = 0; i < 100; i++) {
		if (i!=0) printf(", ");
		printf("%i", jdice_fudge(1));
	}
	putchar('\n');

	printf("Test of +2d20...\n");
	srand(6969);
	printf("%i, %i\n", jdice_ndx(1, 20), jdice_ndx(1, 20));
	srand(6969);
	printf("%i\n", jdice_plus(2, 20));

	printf("Same for -2d20...\n");
	srand(6969);
	printf("%i, %i\n", jdice_ndx(1, 20), jdice_ndx(1, 20));
	srand(6969);
	printf("%i\n", jdice_minus(2, 20));
	return 0;
}
