

all: gui

SRC=gui.c tile50.c
OBJ=$(SRC:%.c=%.o)


gui: $(OBJ)
	FLAGS=`pkg-config --libs hildon-1` || \
	FLAGS=`pkg-config --libs gtk+-2.0`\ -DNOTMAEMO; set -x;\
	gcc -o $@ $(OBJ) $$FLAGS


gui.o: gui.c tile50.h
	sh $<

tile50.o: tile50.c

tile50.c tile50.h: tile50src.c
	perl readrgbc.pl tile50 $<

clean:
	rm -f gui *.o tile50.c tile50.h *~
