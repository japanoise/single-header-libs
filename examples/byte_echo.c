#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#define WDSC_IMPLEMENTATION 1
#include "../screen.h"

int main() {
	char c = 0;;

	printf("This program echoes its input byte-by-byte.\n");
	printf("Press q to quit.\n\n");

	wdsc_init();

	for (;;) {
		c = wdsc_poll();
		if (c == 'q') {
			break;
		} else if (iscntrl(c)) {
			printf("%02x\r\n", (unsigned char) c);
		} else {
			printf("%02x ('%c')\r\n", (unsigned char) c, c);
		}
	}

	wdsc_end();

	return 0;
}
