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
#include <stdlib.h>
#include <string.h>
#include <curses.h>

/* Refresh function; passed the size X and size Y of the screen in cells */
typedef void jap_refresh_func(int, int);

/* Displays a title and a list of choices, and the user can select a choice. */
int jap_choice_index(const char* title, const char** choices, int nchoices,
		 int def);

/*
 * Prompts the user for a string. Allocs/Reallocs the memory, so you
 * must free the returned string. The refresh function is nullable.
 *
 * It doesn't currently support UTF-8, but it should be fairly straightforward
 * to make it do so; see upstream:
 * https://github.com/japanoise/termbox-util/blob/ed9f503/input.go#L44
 */
char* jap_prompt(const char* prompt, jap_refresh_func* refresh_func);

#ifdef _JAP_CURSUTIL_IMP

int jap_choice_index(const char* title, const char** choices, int nchoices,
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

char* jap_prompt(const char* prompt, jap_refresh_func* refresh_func) {
	int bufsize = 0x10;
	int buflen = 0;
	char* buf = calloc(bufsize, 1);
	int bufpos = 0;
	int curs = 0;
	int offset = 0;
	int sx, sy, iw, filler, ch;

	for(;;) {
		getmaxyx(stdscr, sy, sx);
		if (refresh_func != NULL) {
			refresh_func(sx, sy);
		}
		move(sy-1, 0);
		deleteln();
		printw("%s: ", prompt);
		getyx(stdscr, filler, iw);
		while (iw+curs >= sx) {
			offset++;
			curs--;
		}
		while (iw+curs < iw) {
			offset--;
			curs++;
		}
		for (int i = offset; buf[i]!=0; i++) {
			addch(buf[i]);
		}
		move(sy-1, iw+curs);
		refresh();

		ch=getch();
		switch (ch) {
		case KEY_LEFT:
		case 0x02: 	/* ^B */
			if (bufpos > 0) {
				bufpos--;
				curs--;
			}
			break;
		case KEY_RIGHT:
		case 0x06: 	/* ^F */
			if (bufpos < buflen) {
				bufpos++;
				curs++;
			}
			break;
		case KEY_HOME:
		case 0x01: 	/* ^A */
			curs = 0;
			offset = 0;
			bufpos = 0;
			break;
		case KEY_END:
		case 0x05:	/* ^E */
			bufpos=buflen;
			if (buflen>sx) {
				curs = sx-1;
				offset = buflen + 1 - sx;
			} else {
				offset = 0;
				curs = buflen;
			}
			break;
		case '\n':
		case KEY_ENTER:
			return buf;
			break;
		case KEY_DC:
		case 0x04: 	/* ^D */
			/* Move to the right */
			if (bufpos < buflen) {
				bufpos++;
				curs++;
			}
			/* Fallthrough */
		case KEY_BACKSPACE:
		case 0x7F:	/* DEL */
			if (buflen > 0 && bufpos > 0) {
				if (bufpos==buflen) {
					bufpos--;
					buflen--;
					curs--;
					buf[bufpos]=0;
				} else {
					memmove(buf+(bufpos-1), buf+(bufpos),
						buflen-bufpos);
					bufpos--;
					buflen--;
					curs--;
					buf[buflen]=0;
				}
			}
			break;
		case 0x15: 	/* ^U */
			buflen = 0;
			bufpos = 0;
			curs = 0;
			offset = 0;
			buf[0] = 0;
			break;
		case 0x0B:	/* ^K */
			buf[bufpos]=0;
			break;
		default:
			if (bufpos!=buflen) {
				memmove(buf+(bufpos+1), buf+(bufpos), buflen-bufpos);
			}
			buf[bufpos]=ch;
			bufpos++;
			buflen++;
			curs++;
			if (buflen>=bufsize-2) {
				bufsize <<= 1;
				buf=realloc(buf, bufsize);
			}
			buf[buflen]=0;
		}
	}
}

#endif
/* ifdef _JAP_CURSUTIL_IMP */
#endif
/* ifndef _JAP_CURSUTIL_H */
