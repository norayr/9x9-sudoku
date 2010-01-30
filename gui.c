#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -e
 WARN="-Wall -Wno-long-long -Wstrict-prototypes -pedantic"
 WARN="$WARN -Wcast-align -Wpointer-arith " # -Wfloat-equal #-Werror
 WARN="$WARN -W -Wwrite-strings -Wcast-qual -Wshadow" # -Wconversion
 case $1 in '') set x -O2 ### set x -ggdb;
	shift ;; esac;
 FLAGS=`pkg-config --cflags hildon-1` || \
 FLAGS=`pkg-config --cflags gtk+-2.0`\ -DNOTMAEMO
 set -x; ${CC:-gcc} -c -std=gnu99 $WARN "$@" "$0" $FLAGS
 exit $?
 */
#endif
/*
 * $Id; gui.c $
 *
 * Author: Tomi Ollila - too ät iki piste fi
 *
 *	Copyright (c) 2010 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue 26 Jan 2010 18:12:50 EET too
 * Last modified: Sat 30 Jan 2010 21:06:19 EET too
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

extern char ** environ;

#if NOTMAEMO
#define MAEMO 0
#include <gtk/gtk.h>
#else
#define MAEMO 1
#include <hildon/hildon-program.h>
//#include <hildon/hildon-banner.h>
#include <hildon/hildon-gtk.h>
#include <hildon/hildon-main.h>
#endif

#include "lineread.h"
#include "uutil.h"
#include "tile50.h"
#include "up64.h"
#include "down64.h"

typedef enum { false = 0, true = 1 } bool;
#define null ((void*)0)

#if 1
#define dfc(x) do { printf x; } while (0)
#else
#define dfc(x) do {} while (0)
#endif
#define dfc0(x) do {} while (0)

#if MAEMO
gboolean is_portrait(void)
{
    int width = gdk_screen_get_width(W.screen);
    int height = gdk_screen_get_height(W.screen);

    return width < height;
}
#endif

/* globals, except widgets */
struct {
    //const char * prgname;
    LineRead lr;
    GIOChannel * iochannel;
    bool idlehandler;
} G;

/* widgets (and such)*/
struct {
    GdkGC * gc_red;
    GdkGC * gc_white;
    GdkGC * gc_black;

    PangoFontDescription * fd1;
    PangoFontDescription * fd2;
    PangoFontDescription * fd3;
    PangoFontDescription * fd4;
    GdkScreen * screen;
    PangoLayout * layout;
    PangoRenderer * renderer;
} W;

#define DA_HEIGHT 744
#define DA_WIDTH 480

void init_G(void)
{
    memset(&G, 0, sizeof G);
    memset(&W, 0, sizeof W);
}

int run_game_prog(void)
{
    dfc(("run_game_prog\n"));

    int sv[2];
    const char *argv[2] = { "./game.pl", null };

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
	die("socketpair():");

    switch (fork()) {
    case -1:
	die("fork():");
    case 0:
	close(sv[1]);
	move_fd(sv[0], 0);
	dup2(0, 1);
	if (execve(argv[0], argv, environ) != 0)
	    die("execve():");
	break;
    default:
	close(sv[0]);
	return sv[1];
    }
    /* not reached */
    exit(1);
}

void handle_line(char * str, int len)
{
    dfc(("handle_line\n"));
    write(1, str, len);
}

gboolean game_input(void) /* (GIOChannel * source,
			      GIOCondition condition, gpointer data) */
{
    dfc(("game_input: idle %d\n", G.idlehandler));
    char * s;
    int l = lineread(&G.lr, &s);
    if (l < 0)
	exit(1);
    if (l == 0) {
	if (G.idlehandler) {
	    g_io_add_watch(G.iochannel, G_IO_IN | G_IO_HUP, game_input, null);
	    G.idlehandler = false;
	    return false; /* no more idle */
	}
	return true; /* read more data */
    }
    handle_line(s, l);

    if (lineread_count(&G.lr)) {
	if (G.idlehandler)
	    return true; /* more idle */
	else {
	    g_idle_add(game_input, null);
	    G.idlehandler = true;
	    return false; /* no more io */
	}
    }
    else {
	lineread(&G.lr, &s); /* set 'select()ed' lineread state */
	if (G.idlehandler) {
	    g_io_add_watch(G.iochannel, G_IO_IN | G_IO_HUP, game_input, null);
	    G.idlehandler = false;
	    return false; /* no more idle */
	}
	return true; /* read more data */
    }
}

void start_game(void)
{
    dfc(("start_game\n"));

    int fd = run_game_prog();

    dfc(("fd %d\n", fd));
    lineread_init(&G.lr, fd);

    G.iochannel = g_io_channel_unix_new(fd);
    g_io_add_watch(G.iochannel, G_IO_IN | G_IO_HUP, game_input, null);
}

/* screen ui data, can be created many times during program execution */
struct {
    struct {
	gint16 x;
	gint16 y;
	gint8 value;
	gint8 notes[9];
    } table[9][9];
    struct {
	gint16 left;
	gint16 right;
	gint16 up;
	gint16 down;
    } tlimits[9];

    struct {
	gint16 x;
	gint16 y;
	gint8 state;
	char value;
    } buttons[5][2];

    struct {
	gint16 left;
	gint16 right;
    } blimits_x[5];
    struct {
	gint16 up;
	gint16 down;
    } blimits_y[2];
} T;

void init_tables(void)
{
    memset(&T, 0, sizeof T);

    for (int i = 0; i < 9; i++) {
	int x = 00 + 4 + i * 52 + 3 * (i / 3);
	for (int j = 0; j < 9; j++) {
	    int y = 50 + 4 + j * 52 + 3 * (j / 3);

	    T.table[i][j].x = x;
	    T.table[i][j].y = y;
	    if (i == 0) {
		T.tlimits[j].up = y + 1;
		T.tlimits[j].down = y + 48;
	    }
	}
	T.tlimits[i].left = x + 1;
	T.tlimits[i].right = x + 48;
    }

    for (int i = 0; i < 5; i++) {
	int x = 64 + i * 72;
	for (int j = 0; j < 2; j++) {
	    int y = 50 + 520 + j * 72;

	    T.buttons[i][j].x = x;
	    T.buttons[i][j].y = y;
	    T.buttons[i][j].value = '1' + i + 5 * j;
	    if (i == 0) {
		T.blimits_y[j].up = y + 1;
		T.blimits_y[j].down = y + 62;
	    }
	}
	T.blimits_x[i].left = x + 1;
	T.blimits_x[i].right = x + 62;
    }
    T.buttons[4][1].value = ' ';
}

void init_draw(GtkWidget * widget)
{
    GdkDrawable * drawable = widget->window;

    W.gc_black = gdk_gc_new(drawable);
    W.gc_white = gdk_gc_new(drawable);
    W.gc_red = gdk_gc_new(drawable);

    GdkColor color = { .red = 32768, .green = 32768, .blue = 65535 };
    gdk_gc_set_rgb_fg_color(W.gc_white, &color);
    color.green = color.blue = 0;
    gdk_gc_set_rgb_fg_color(W.gc_red, &color);
    color.red = 0;
    gdk_gc_set_rgb_fg_color(W.gc_black, &color);

    W.screen = gdk_screen_get_default();
    W.renderer = gdk_pango_renderer_get_default(W.screen);
    gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(W.renderer), drawable);

    W.layout = gtk_widget_create_pango_layout(widget, "");

    W.fd1 = pango_font_description_from_string("Monospace bold 12");
    W.fd2 = pango_font_description_from_string("Monospace bold 18");
    W.fd3 = pango_font_description_from_string("Monospace bold 24");
    W.fd4 = pango_font_description_from_string("Monospace bold 36");
}

void render_text(int x, int y, PangoFontDescription * fd, GdkGC * gc, char * t)
{
    pango_layout_set_font_description (W.layout, fd);
    gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(W.renderer), gc);

    pango_layout_set_text (W.layout, t, strlen(t));

    pango_renderer_draw_layout (W.renderer, W.layout,
				x * PANGO_SCALE, y * PANGO_SCALE);
}

void draw_char(int x, int y, char c)
{
    pango_layout_set_text (W.layout, &c, 1);

    pango_renderer_draw_layout (W.renderer, W.layout,
				x * PANGO_SCALE, y * PANGO_SCALE);
}

gboolean darea_expose(GtkWidget * w, GdkEventExpose * e, gpointer user_data)
{
    (void)e; (void)user_data;

    int i;
    int j;

#if MAEMO
    if (! is_portrait() )
	return;
#else
    gdk_draw_rectangle(w->window, W.gc_black, true, 0, 0, DA_WIDTH, DA_HEIGHT);
#endif

    pango_layout_set_font_description (W.layout, W.fd1);
    gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(W.renderer), W.gc_black);

    for (i = 0; i < 9; i++) {
	for (j = 0; j < 9; j++) {
	    int x = T.table[i][j].x;
	    int y = T.table[i][j].y;

	    gdk_draw_rgb_image(w->window, W.gc_white, x, y, 50, 50, 0,
			       tile50_pixel_data, 150);
	    //gdk_draw_rectangle(w->window, W.gc_white, true, x, y, 50, 50);

	    if (i == 6) {
		pango_layout_set_font_description (W.layout, W.fd3);
		draw_char(x + 15, y + 7, j + '1');
	    }
	    else {
		pango_layout_set_font_description (W.layout, W.fd1);
		draw_char(x + 6, y + 1, '1');
		draw_char(x + 20, y + 1, '2');
		draw_char(x + 34, y + 1, '3');

		draw_char(x + 6, y + 16, '4');
		draw_char(x + 20, y + 16, '5');
		draw_char(x + 34, y + 16, '6');

		draw_char(x + 6, y + 31, '7');
		draw_char(x + 20, y + 31, '8');
		draw_char(x + 34, y + 31, '9');
	    }
	}
    }

    pango_layout_set_font_description (W.layout, W.fd2);
    gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(W.renderer), W.gc_black);

    for (i = 0; i < 5; i++)
	for (j = 0; j < 2; j++) {
	    int x = T.buttons[i][j].x;
	    int y = T.buttons[i][j].y;

	    int ox, oy;
	    const unsigned char * pd;
	    if (i == 3) {
		pango_layout_set_font_description (W.layout, W.fd2);
		pd = down64_pixel_data;
		ox = x + 24; oy = y + 18;
	    }
	    else {
		pango_layout_set_font_description (W.layout, W.fd4);
		pd = up64_pixel_data;
		ox = x + 18; oy = y + 4;
	    }

	    gdk_draw_rgb_image(w->window, W.gc_white, x, y, 64, 64, 0,
			       pd, 192);

	    //gdk_draw_rectangle(w->window, W.gc_red, true, x, y, 64, 64);
	    draw_char(ox, oy, T.buttons[i][j].value);
	    //pango_layout_set_font_description (W.layout, W.fd3);
	}

    printf("exposed\n");
    return true;
}

gboolean darea_button_press(GtkWidget * w, GdkEventButton * e, gpointer ud)
{
    (void)w; (void)ud;
    int x = (int)e->x;
    int y = (int)e->y;

    int r = -1, c = -1;

    if (y > T.tlimits[0].up && y < T.tlimits[8].down)
    {
	//printf("%d %d\n", T.tlimits[0].left, T.tlimits[0].right);
	//printf("%d %d\n", T.tlimits[0].up, T.tlimits[0].down);

	for (r = 8; r >= 0; r--)
	    if ( x > T.tlimits[r].left && x < T.tlimits[r].right)
		break;

	for (c = 8; c >= 0; c--)
	    if ( y > T.tlimits[c].up && y < T.tlimits[c].down)
		break;

	if (c >= 0 && r >= 0)
	    fdprintf1k(G.lr.fd, "# %d %d\n", r, c);
    }
    else if (y > T.blimits_y[0].up && y < T.blimits_y[1].down)
    {
	for (r = 4; r >= 0; r--)
	    if ( x > T.blimits_x[r].left && x < T.blimits_x[r].right)
		break;

	for (c = 1; c >= 0; c--)
	    if ( y > T.blimits_y[c].up && y < T.blimits_y[c].down)
		break;
	if (c >= 0 && r >= 0)
	    fdprintf1k(G.lr.fd, "* %d %d\n", r, c);
    }

    //printf("buttonpress: %d %d (%d %d)\n", x, y, r, c);
    return true;
}

void darea_realize(GtkWidget * w, gpointer user_data)
{
    (void)user_data;
    init_draw(w);

    gtk_widget_add_events(w, GDK_BUTTON_PRESS_MASK);

    printf("realized\n");

    gtk_signal_connect(GTK_OBJECT(w), "expose-event",
		       GTK_SIGNAL_FUNC(darea_expose), null);

    gtk_signal_connect(GTK_OBJECT(w), "button-press-event",
		       GTK_SIGNAL_FUNC(darea_button_press), null);

}



void save_and_quit(void)
{
    // script will save it's state when fd closes
    gtk_main_quit();
}

/* http://talk.maemo.org/showthread.php?p=461531 */

void buildgui(void)
{
    init_tables();

    /* Create the main window */
#if MAEMO
    GtkWidget * mainwin = hildon_stackable_window_new();
#else
    GtkWidget * mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif

    gtk_window_set_title(GTK_WINDOW(mainwin), "Thumb Sudoku");
    g_signal_connect(G_OBJECT(mainwin), "delete_event",
                     G_CALLBACK(save_and_quit), null);

#if 0 // XXX this might be needed...
    g_signal_connect(screen, "size-changed", on_orientation_changed, widget);
#endif

#if MAEMO
    //gtk_window_fullscreen (GTK_WINDOW (mainwin));// will make title disappear
    // portrait mode
    hildon_gtk_window_set_portrait_flags(GTK_WINDOW(mainwin),
					 HILDON_PORTRAIT_MODE_REQUEST);
#endif

    GtkWidget * da = gtk_drawing_area_new();
    gtk_widget_set_size_request(da, DA_WIDTH, DA_HEIGHT);
    gtk_signal_connect(GTK_OBJECT(da), "realize",
		       GTK_SIGNAL_FUNC(darea_realize), null);

    gtk_container_add(GTK_CONTAINER(mainwin), da);

    /* Show the application window */
    gtk_widget_show_all (mainwin);
}


int main(int argc, char * argv[])
{
    init_G();
    /* Initialize i18n support */
    gtk_set_locale ();

    /* Initialize the widget set */
#if MAEMO
    hildon_gtk_init (&argc, &argv);
#else
    gtk_init(&argc, &argv);
#endif

    buildgui();

    start_game();

    /* Enter the main event loop, and wait for user interaction */
    gtk_main ();

    /* The user lost interest */
    return 0;
}
