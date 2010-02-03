

all: gui

SRC=gui.c lineread.c uutil.c tile50.c up64.c down64.c
OBJ=$(SRC:%.c=%.o)


gui: $(OBJ)
	FLAGS=`pkg-config --libs hildon-1 libosso` || \
	FLAGS=`pkg-config --libs gtk+-2.0`\ -DNOTMAEMO; set -x;\
	gcc -o $@ $(OBJ) $$FLAGS

FILE_PNG64 = thumb-sudoku-64.png
FILE_DESKTOP = thumb-sudoku.desktop
FILE_SERVICE = thumb-sudoku.service

install_maemo:
	mkdir -p $(DESTDIR)/usr/share/icons/hicolor/64x64/apps
	cp $(FILE_PNG64) $(DESTDIR)/usr/share/icons/hicolor/64x64/apps/ma	
	mkdir -p $(DESTDIR)/usr/share/applications/hildon
	cp $(FILE_DESKTOP) $(DESTDIR)/usr/share/applications/hildon
	mkdir -p $(DESTDIR)/usr/share/dbus-1/services
	cp $(FILE_SERVICE) $(DESTDIR)/usr/share/dbus-1/services
	mkdir -p $(DESTDIR)/opt/maemo/usr/lib/thumb_sudoku
	cp gui game.pl precalc \
		$(DESTDIR)/opt/maemo/usr/lib/thumb_sudoku

gui.o: gui.c tile50.h up64.h down64.h
	sh $<

uutil.o: uutil.c
	sh $<

lineread.o: lineread.c
	sh $<

tile50.o: tile50.c

tile50.c tile50.h: tile50src.c
	perl readrgbc.pl tile50 $<

up64.c up64.h: up64src.c
	perl readrgbc.pl up64 $<

down64.c down64.h: down64src.c
	perl readrgbc.pl down64 $<

clean distclean:
	rm -f gui *.o tile50.c tile50.h up64.c up64.h down64.c down64.h *~
	-debian/rules clean

mad_srcpkg: # Published MADDE images do not (yet) have 'fakeroot' command...
	mad dpkg-buildpackage -sa -S -I.git

bumprev: # 0ld Amiga term...
	perl -pi -e 'BEGIN { chomp($$date=`date -R`); $$set = 0; } \
	 s/(\d+)\.(\d+)/sprintf "$$1.%d",$$2 + 1/e unless $$set; \
	 s/>.*/>  $$date/, $$set = 1 if (/--.*>/ && !$$set);' debian/changelog
