#include <curses.h>
#define _JAP_CURSUTIL_IMP 1
#include "../jap_cursutil.h"

int main(int argc, char *argv[]) {
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	char* name = jap_prompt("What's your name?", NULL);

	char* title = "Which fruit do you like?";
	const char *choices[46];
	choices[0] = "Loganberry";
	choices[1] = "Orange";
	choices[2] = "Mango";
	choices[3] = "Slime-mold";
	choices[4] = "Avocado";
	choices[5] = "Akee";
	choices[6] = "Araza";
	choices[7] = "Alibertia";
	choices[8] = "Apricot";
	choices[9] = "Blueberry";
	choices[10] = "Blackberry";
	choices[11] = "Barberry";
	choices[12] = "Banana";
	choices[13] = "Bignay";
	choices[14] = "Biriba";
	choices[15] = "Breadfruit";
	choices[16] = "Barbados Cherry";
	choices[17] = "Bael Fruit";
	choices[18] = "Black Sapote";
	choices[19] = "Bilberry";
	choices[20] = "Boysenberry";
	choices[21] = "Blackcurrant";
	choices[22] = "Champedak";
	choices[23] = "Custard Apple";
	choices[24] = "Chalta";
	choices[25] = "Calabash";
	choices[26] = "Cape Gooseberry";
	choices[27] = "Ceylon Gooseberry";
	choices[28] = "Clementine";
	choices[29] = "Cherry";
	choices[30] = "Canary Melon";
	choices[31] = "Cantaloupe";
	choices[32] = "Casaba";
	choices[33] = "Crenshaw";
	choices[34] = "Charantais";
	choices[35] = "Christmas Melon";
	choices[36] = "Cranberry";
	choices[37] = "Currants";
	choices[38] = "Cucumber";
	choices[39] = "Date";
	choices[40] = "Durian";
	choices[41] = "Feijoa";
	choices[42] = "Fig";
	choices[43] = "Gandaria";
	choices[44] = "Golden Apple";
	choices[45] = "Genipap";
	int choice = jap_choice_index(title, choices, 46, 0);

	curs_set(0);
	clear();
	if (choice == 3)
		mvprintw(0, 0, "Nice, %s! We have matching choices. We're pals.", name);
	else
		mvprintw(0, 0, "You're weird, %s!", name);
	refresh();
	getch();
	endwin();
	free(name);
	return 0;
}
