all: a.out
	./a.out sample-html/00*
 
a.out: main.c $(wildcard src/*.c) $(wildcard src/*.h)
	gcc -I /usr/lib/x86_64-linux-gnu/glib-2.0/include -I /usr/include/glib-2.0 -I ./src -g $^ -lgumbo -lpanel -lncurses -lm -lpthread `pkg-config --cflags --libs glib-2.0`

clean:
	rm a.out
	rm src/*.gch
	