CC=clang

PROGS=main

CFLAGS=`pkg-config --cflags sdl2 gl glew libwacom libinput`
LDFLAGS=`pkg-config --libs sdl2 gl glew libwacom libinput`

all: $(PROGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) ${CFLAGS} $< -o $@

%: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) ${CFLAGS} ${LDFLAGS} $^ -o $@

clean:
	rm -f $(PROGS) *.o
