#ifndef WD_SCREEN_H
#define WD_SCREEN_H

/*
 * # SCREEN.H
 *
 * This is a very thin wrapper around ANSI escape codes; especially as
 * implemented by the VT200 and modern terminal emulators. This means
 * that it should "just work" under most unix-likes, and work decently
 * in emulations thereof (such as Cygwin or ANSI.SYS), but comes with
 * no guarantees. It "works on my machine," and should work with
 * anything similar to xterm (I've tested it with st, xterm, and
 * lxterminal).
 *
 * If you want to find out what byte sequence you should use for
 * e.g. Home and End (most likely ESC-something), use the byte_echo
 * program, the source code of which should be provided with this
 * file.
 *
 * ## USAGE
 *
 * As always for single-header-libraries, define WDSC_IMPLEMENTATION
 * in one of your source files before `#include "screen.h"` to
 * include the implementation of this library.
 *
 * Run wdsc_init() before you do anything; wdsc_end() when you're done.
 *
 * Get the terminal size with the wdsc_screensize(&x, &y)
 * function. Beware, this is a little slow, so cache those values
 * somewhere (a global is perfectly acceptable). See below for
 * handling terminals being resized.
 *
 * To grab a byte of input, call wdsc_poll().
 *
 * To move the cursor around, call wdsc_set_cursor(x, y). Note indices
 * start from 1, 1 at the top left corner of the terminal, and grow
 * larger as you go to the bottom right.
 *
 * Most terminals should allow you to hide and show the cursor with
 * wdsc_hide_cursor() and wdsc_show_cursor() respectively.
 *
 * To clear the screen, call wdsc_clear().
 *
 * To print something on the screen, call wdsc_set(x, y, c) for a char,
 * or wdsc_puts(x, y, s) for a string.
 *
 * To draw things in color or with special attributes, call
 * wdsc_attr_on(a) or wdsc_attr_on_s(s). See
 * https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_parameters for
 * codes you can use for attributes. Once you're done, call
 * wdsc_attr_off().
 *
 * **PLEASE DO NOT ASSUME YOUR USER IS USING WHITE ON BLACK OR BLACK
 * ON WHITE FOR THEIR TERMINAL. "BLACK" MAY BE DIFFERENT TO THE
 * FOREGROUND, "WHITE" MAY BE DIFFERENT TO THE BACKGROUND, AND VICE
 * VERSA. TO GO BACK TO NORMAL, CALL wdsc_attr_off()! IF YOU DO NOT,
 * YOUR USERS MAY EXPERIENCE GRAPHICAL GLITCHES OR EVEN ACCESSIBILITY
 * ISSUES.**
 *
 * Once you're done manipulating the screen, call wdsc_present() to
 * flush the buffers and present the screen.
 *
 * ### RESIZING THE TERMINAL
 *
 * If the terminal size changes, the program will be sent the signal
 * SIGWINCH. This library delegates handling this to the user. Since
 * the default behavior is to just ignore this signal, you can safely
 * set it up to run some custom code instead.
 *
 * I recommend a function pointer variable (global) for the current
 * draw function, which should take size x and y, or load them from
 * your cache. Before calling this function, update your cache by
 * calling wdsc_screensize(&x, &y). Trivial example:
 *
 * ```
 * #include<signal.h>
 *
 * int sx;
 * int sy;
 * void (*draw)();
 *
 * void my_draw_func() {
 *     ...
 * }
 *
 * void sigwinch(int signal) {
 *     wdsc_screensize(&sx, &sy);
 *     draw()
 * }
 *
 * int main() {
 *     wdsc_init();
 *     wdsc_screensize(&sx, &sy);
 *     draw = my_draw_func;
 *     signal(SIGWINCH, sigwinch);
 *     ...
 *     wdsc_end();
 *     return 0;
 * }
 * ```
 *
 * ## COPYING
 *
 * MIT LICENSE:
 *
 * Copyright © 2020 japanoise
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

void wdsc_init();

void wdsc_end();

void wdsc_set(int x, int y, char c);

void wdsc_puts(int x, int y, const char *s);

void wdsc_set_cursor(int x, int y);

void wdsc_clear();

void wdsc_present();

void wdsc_attr_on(int n);

void wdsc_attr_on_s(char * s);

void wdsc_attr_off();

char wdsc_poll();

void wdsc_screensize(int *x, int *y);

void wdsc_hide_cursor();

void wdsc_show_cursor();

#ifdef WDSC_IMPLEMENTATION
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "screen.h"

static void die(const char *s) {
	wdsc_end();
	perror(s);
	exit(1);
}

/* Some macro fun for shortcuts; I'm too lazy to write out shit in full. */

/* Quick putc (avoid typing out stdout) */
#define QPUTC(c) fputc(c, stdout)

/* Print ESC */
#define ESC QPUTC(033)

/* Print the CSI sequence ESC [ */
#define CSI ESC; QPUTC('[')

/* Call CUP quickly with two chars */
#define CUPRICE(x, y) CSI; QPUTC(y); QPUTC(;) QPUTC(x); QPUTC('H')

/* Flush stdout */
#define DOFLUSH fflush(stdout)

/* Print an m (tail of SGR) */
#define SGR_TAIL QPUTC('m');

static struct termios orig_termios;

/* Thanks, antirez's kilo. */
static void enableRawMode() {
	struct termios raw;
	if (tcgetattr(STDIN_FILENO, &raw) == -1) die("tcgetattr");
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

static void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void wdsc_end() {
	disableRawMode();
}

static void csi_cup(int x, int y) {
	CSI;
	fprintf(stdout, "%i;%iH", y, x);
}

void wdsc_init() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
		die("tcsetattr");
	enableRawMode();
	DOFLUSH;
	atexit(disableRawMode);
}

void wdsc_present() {
	DOFLUSH;
}

void wdsc_set(int x, int y, char c) {
	/* Save cursor position */
	ESC;
	QPUTC('7');

	/* Jump to X, Y */
	csi_cup(x, y);

	/* Draw character */
	QPUTC(c);

	/* Restore cursor position */
	ESC;
	QPUTC('8');
}

void wdsc_puts(int x, int y, const char *s) {
	/* Save cursor position */
	ESC;
	QPUTC('7');

	/* Jump to X, Y */
	csi_cup(x, y);

	/* Print the string. */
	fprintf(stdout, "%s", s);

	/* Restore cursor position */
	ESC;
	QPUTC('8');
}

void wdsc_set_cursor(int x, int y) {
	csi_cup(x, y);
}

void wdsc_attr_on(int n) {
	CSI;
	fprintf(stdout, "%i", n);
	SGR_TAIL;
}

void wdsc_attr_on_s(char * s) {
	CSI;
	fprintf(stdout, "%s", s);
	SGR_TAIL;
}

void wdsc_attr_off() {
	CSI;
	QPUTC('0');
	SGR_TAIL;
}

void wdsc_clear() {
	CSI;
	QPUTC('2');
	QPUTC('J');
}

void wdsc_screensize(int *x, int *y) {
	/* Save cursor position */
	ESC;
	QPUTC('7');

	/* Jump very very far to the bottom right */
	csi_cup(999, 999);

	/* Request the cursor position */
	CSI;
	QPUTC('6');
	QPUTC('n');

	/* Flush, so we don't get caught here */
	DOFLUSH;

	/* Snarf the terminal's response */
	char ecks[5];
	char why[5];
	for (int i = 0; i < 5; i++) {
		ecks[i] = 0;
		why[i] = 0;
	}
	int idx = 0;
	bool snarfingX = false;
	for(;;) {
		char c = wdsc_poll();
		if (c == 033 || c == '[') {
			/* do nothing */
		} else if (c == ';') {
			snarfingX = true;
			idx = 0;
		} else if (c == 'R') {
			break;
		} else if (snarfingX) {
			ecks[idx++] = c;
		} else {
			why[idx++] = c;
		}
	}

	/* Update the variables */
	*x = atoi(ecks);
	*y = atoi(why);

	/* Restore cursor position */
	ESC;
	QPUTC('8');
}

char wdsc_poll() {
	char c;
	while (read(STDIN_FILENO, &c, 1) != 1);
	return c;
}


void wdsc_hide_cursor() {
	CSI;
	QPUTC('?');
	QPUTC('2');
	QPUTC('5');
	QPUTC('l');
}

void wdsc_show_cursor() {
	CSI;
	QPUTC('?');
	QPUTC('2');
	QPUTC('5');
	QPUTC('h');
}
#endif
#endif
