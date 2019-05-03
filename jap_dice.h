/*
 * Dice Library
 * ============
 *
 * A simple library to roll dice for tabletop role-playing games.
 * You can call the functions directly, or use the parser.
 *
 * One of your objects must contain the implementation; #define the symbol
 * JAP_DICE_IMP before including this file.
 *
 * You can also define your own random number function for this library to use
 * by defining the symbol JAP_RAND(n). Note that it must accept an int and
 * return an int X in the range 0 <= X < n. Alternatively, don't define it and
 * get a bog standard call to rand().
 *
 * This file is licensed under the MIT License; see the file LICENSE for
 * details.
 *
 * The parser understands the following dice formats:
 *
 * - NdX - Standard format. Roll N X-Sided dice, return the sum of the dice.
 *   E.G. 2d10 = roll two ten-sided dice and take the result.
 * - ND - Used in Traveller and some other old games. Roll N 6-sided dice and
 *   return the sum of the dice.
 *   E.G. 3D - equiv. to 3d6.
 * - NdF|Ndf - Fudge dice. Roll dice with +, -, and blank sides, and return
 *   the number of pluses (if positive) or number of minuses (if negative).
 *   E.G. 4df gives us ++-0. Return 1. A second roll gives us --+0. Return -1.
 * - +NdX - Whitehack. Roll N X-Sided dice, take the best result.
 *   E.G. +2d20 - we get a 14 and a 7, so return 14.
 * - -NdX - Whitehack. Roll N X-Sided dice, take the worst result.
 *   E.G. -2d20 - we get a 14 and a 7, so return 7.
 *
 * The parser prioritises speed over accuracy, so has some limitations:
 *
 * - Garbage in, garbage out - see the example program.
 * - Errors return the special value JAP_DICE_PARSERR.
 * - The parser only understands positive decimal integers.
 * - It follows the philosophy of wanting to return as soon as possible.
 * - It must be passed a proper null-terminated C-string or bad things will
 *   happen.
 *
 * Numbers parsed are limited to JAP_DICE_MAX (default 1000) to
 * prevent overflows.
 *
 * To cache parser results, use the struct jap_diceroll. This is also
 * more reliable with errors.
 */

#ifndef _JAP_DICE_H
#define _JAP_DICE_H 1

typedef enum {DNDX, DFUDGE, DMAX, DMIN} jap_dice_type;

typedef struct {
	jap_dice_type type;
	int n;
	int x;
} jap_diceroll;

/* Roll NdX and return the sum */
int jdice_ndx(int n, int x);

/* Roll NdF and return the number of pluses (positive)/minuses (negative) */
int jdice_fudge(int n);

/* Roll NdX and return the best roll */
int jdice_max(int n, int x);

/* Roll NdX and return the worst roll */
int jdice_min(int n, int x);

/* Parse the string s and put the parse result in roll; return 0 on
 * success, JAP_DICE_PARSERR otherwise. roll is nullable; if roll=null, the
 * function will do a dry run. */
int jdice_parse(const char* s, jap_diceroll* roll);

/* Roll the dice described in roll */
int jdice_roll(jap_diceroll* roll);

/* Parse the string s and roll immediately. Return JAP_DICE_PARSERR if
 * parser error. */
int jdice_parse_and_roll(const char* s);

#define JAP_DICE_PARSERR -6969

#ifdef JAP_DICE_IMP

#include <limits.h>
#include <stdbool.h>

#ifndef JAP_DICE_MAX
#define JAP_DICE_MAX 1000
#endif	/* JAP_DICE_MAX */

#ifndef JAP_RAND
#include <stdlib.h>
#define JAP_RAND(x) (rand()%x)
#endif	/* JAP_RAND */

int jdice_ndx(int n, int x) {
	int result = 0;
	for (int i = 0; i < n; i++) {
		result += JAP_RAND(x)+1;
	}
	return result;
}

int jdice_fudge(int n) {
	int result = 0;
	for (int i = 0; i < n; i++) {
		result += JAP_RAND(3)-1;
	}
	return result;
}

int jdice_max(int n, int x) {
	int result = 0;
	for (int i = 0; i < n; i++) {
		int roll = JAP_RAND(x)+1;
		if (roll > result)
			result = roll;
	}
	return result;
}

int jdice_min(int n, int x) {
	int result = INT_MAX;
	for (int i = 0; i < n; i++) {
		int roll = JAP_RAND(x)+1;
		if (roll < result)
			result = roll;
	}
	return result;
}

// Internal helper function: asserts that the string contains only whitespace.
// Returns 0 on success, or else JAP_DICE_PARSERR.
int jdice__whitespace_or_error(const char* s) {
	char c;
	while ((c = *s++) != 0) {
		switch (c) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				break;
			default:
				return JAP_DICE_PARSERR;
		}
	}

	return 0;
}

int jdice_parse(const char* s, jap_diceroll* roll) {
	int num = 0;
	bool seenn = false;
	bool firstchr = true;
	jap_dice_type type = DNDX;
	for (const char* str = s; *str!=0; str++) {
		char ch = *str;
		bool whitespace = false;
		switch (ch) {
		case '+':
			if (!firstchr) return JAP_DICE_PARSERR;
			type = DMAX;
			break;
		case '-':
			if (!firstchr) return JAP_DICE_PARSERR;
			type = DMIN;
			break;
		case 'D':
			if (roll!=NULL) {
				roll->type = type;
				roll->x = 6;
			}
			/* fallthrough */
		case 'd':
			if(num == 0 || num > JAP_DICE_MAX || seenn) return JAP_DICE_PARSERR;

			if (roll!=NULL) roll->n = num;
			seenn = true;
			num = 0;
			break;
		case 'f':
		case 'F':
			if (!seenn || type != DNDX) return JAP_DICE_PARSERR;
			if (roll != NULL) {
				roll->type=DFUDGE;
			}
			return jdice__whitespace_or_error(str + 1);
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			/* Ignore whitespace */
			whitespace = true;
			break;
		default:
			if ('0' <= ch && ch <= '9') {
				num = (10 * num) + (ch - '0');
			} else {
				return JAP_DICE_PARSERR;
			}
		}
		if (ch=='D') return jdice__whitespace_or_error(str + 1);
		if (!whitespace) firstchr = false;
	}
	if(num == 0 || num > JAP_DICE_MAX || !seenn) return JAP_DICE_PARSERR;

	if (roll!=NULL) {
		roll->x = num;
		roll->type = type;
	}

	return 0;
}

int jdice_roll(jap_diceroll* roll) {
	switch (roll->type) {
	case DNDX:
		return jdice_ndx(roll->n, roll->x);
	case DFUDGE:
		return jdice_fudge(roll->n);
	case DMAX:
		return jdice_max(roll->n, roll->x);
	default:
		return jdice_min(roll->n, roll->x);
	}
}

int jdice_parse_and_roll(const char* s) {
	jap_diceroll roll;
	int res = jdice_parse(s, &roll);
	if (res)
		return res;
	return jdice_roll(&roll);
}

#endif	/* JAP_DICE_IMP */
#endif	/* _JAP_DICE_H */
