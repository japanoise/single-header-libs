EXE = cursutil dice

all: $(EXE)

cursutil: cursutil.c ../jap_cursutil.h
	$(CC) cursutil.c -lncurses -o$@

dice: dice.c ../jap_dice.h
	$(CC) dice.c -o$@

clean:
	rm -rf $(EXE)
