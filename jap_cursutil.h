/*
 * Curses utility functions
 * ========================
 *
 * Port of termbox-util to C and curses, with some interface changes.
 * Not yet really ready for the prime-time, but you're free to try it out!
 *
 * Make sure one of your object files includes this header and defines the
 * symbol _JAP_CURSUTIL_IMP before that, to include the implementation.
 *
 * This file is licensed under the MIT License; see the file LICENSE for
 * details.
 */

#ifndef _JAP_CURSUTIL_H
#define _JAP_CURSUTIL_H 1
#include <curses.h>

/* Displays a title and a list of choices, and the user can select a choice. */
int choice_index(const char* title, const char** choices, int nchoices,
		 int def);

#ifdef _JAP_CURSUTIL_IMP

int choice_index(const char* title, const char** choices, int nchoices,
		 int def) {
	int choice = def;
	int offset = 0;
	int sx, sy, ch;

        for(;;) {
		clear();
		getmaxyx(stdscr, sy, sx);
		mvinsstr(0, 0, title);
		while (choice < offset) {
			offset -=5;
			if (offset < 0) offset=0;
		}
		while (choice-offset >= sy-1) {
			offset +=5;
			if (offset >= nchoices)
				offset = nchoices;
		}

		int ind = 0;
		for (int i = offset; i < nchoices; i++) {
			mvinsstr(1+ind, 2, choices[i]);
			ind++;
		}

		mvaddch((choice+1)-offset, 0, '>');
		move((choice+1)-offset, 2);
		refresh();

	dontredraw:
		ch = getch();
		switch (ch) {
		case 'k':
		case KEY_UP:
		case 0x10: /* ^P */
			if (1 <= choice) choice--;
			break;
		case 'j':
		case KEY_DOWN:
		case 0x0E: /* ^N */
			if (choice < nchoices-1) choice++;
			break;
		case '\n':
		case KEY_ENTER:
			return choice;
		default:
			goto dontredraw;
		}
	}
}

#endif
/* ifdef _JAP_CURSUTIL_IMP */
#endif
/* ifndef _JAP_CURSUTIL_H */
