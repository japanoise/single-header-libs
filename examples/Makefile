EXE = cursutil

all: $(EXE)

cursutil: cursutil.c ../jap_cursutil.h
	$(CC) cursutil.c -lncurses -o$@

clean:
	rm -rf $(EXE)
