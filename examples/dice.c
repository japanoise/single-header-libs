#include <stdio.h>
#include <stdlib.h>
#define JAP_DICE_IMP 1
#include "../jap_dice.h"

int parsetest(char* dice) {
	jap_diceroll roll;
	int res;

	printf("Parsing %s...\n", dice);
	if (jdice_parse(dice, NULL)) {
		printf("Failed parser dry run for %s\n", dice);
	}
	res = jdice_parse(dice, &roll);
	if (res) {
		printf("Couldn't parse %s\n", dice);
		return 1;
	}
	printf("Parse result: n=%i, x=%i, type=%i\n", roll.n, roll.x, roll.type);
	printf("Rolled: %i\n", jdice_roll(&roll));
	return 0;
}

void print_roll(int n) {
	printf("%i, ", n);
}

void print_roll_signed(int n) {
	printf("%+i, ", n);
}

int main() {
	jap_diceroll roll;
	int res;

	srand(6969);
	printf("These should parse successfully.\n");
	parsetest("2d10");
	parsetest("3D");
	parsetest("4dF");
	parsetest("4df");
	parsetest("+2d20");
	parsetest("-3d10");
	printf("\nThese should fail.\n");
	parsetest("2d3d4");
	parsetest("2D3d4");
	parsetest("FdF");
	parsetest("28F");
	parsetest("-5dF");
	parsetest("5DF");
	parsetest("+-+--+-4d3");
	parsetest("34+d2");
	parsetest("3-4d2");
	parsetest("33dF2");
	parsetest("3");

	printf("\nI'm going to roll 100d6.\n");
	jdice_parse("100d6", &roll);
	res = jdice_roll_func(&roll, print_roll);
	printf("total: %i\n", res);

	printf("\nNow let's try 10dF.\n");
	jdice_parse("10dF", &roll);
	res = jdice_roll_func(&roll, print_roll_signed);
	printf("total: %i\n", res);

	printf("\nTest of +2d20...\n");
	srand(6969);
	jdice_parse("+2d20", &roll);
	res = jdice_roll_func(&roll, print_roll);
	printf("result: %i\n", res);

	printf("\nSame for -2d20...\n");
	srand(6969);
	jdice_parse("-2d20", &roll);
	res = jdice_roll_func(&roll, print_roll);
	printf("result: %i\n", res);
	return 0;
}
