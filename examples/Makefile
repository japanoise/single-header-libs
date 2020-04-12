EXE = cursutil dice byte_echo

all: $(EXE)

byte_echo: byte_echo.c ../screen.h
	$(CC) byte_echo.c -o$@

cursutil: cursutil.c ../jap_cursutil.h
	$(CC) cursutil.c -lncurses -o$@

dice: dice.c ../jap_dice.h
	$(CC) dice.c -o$@

clean:
	rm -rf $(EXE)
