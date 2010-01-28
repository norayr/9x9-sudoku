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
 * Last modified: Thu 28 Jan 2010 20:50:06 EET too
 */

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

#include "tile50.h"

enum { false = 0, true = 1 } bool;
#define null ((void*)0)

/* http://talk.maemo.org/showthread.php?p=461531 */

#if MAEMO
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

/* Rotate window into portrat mode and make fullscreen */

void portraitmode(Window win)
{
    unsigned char on = 1;
    Display * dpy = XOpenDisplay(0/*null*/);/* or use toolkit's dpy variable */
    Atom newstate;

    XChangeProperty(dpy, win,
		    XInternAtom(dpy, "_HILDON_PORTRAIT_MODE_SUPPORT", True),
		    XA_CARDINAL, 32, PropModeReplace, &on, 1);

    XChangeProperty(dpy, win,
		    XInternAtom(dpy, "_HILDON_PORTRAIT_MODE_REQUEST", True),
		    XA_CARDINAL, 32, PropModeReplace, &on, 1);

    newstate = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(dpy, win,
		    XInternAtom(dpy, "_NET_WM_STATE", True), XA_ATOM, 32,
		    PropModeReplace, (unsigned char *) &newstate, 1);
}
#endif

/* globals, except widgets */
struct {
    int table[9][9];
} G;

/* widgets (and such)*/
struct {
    GdkGC * gc_red;
    GdkGC * gc_white;
    GdkGC * gc_black;
} W;

#define DA_HEIGHT 640
#define DA_WIDTH 480


void drawstuff(GdkDrawable * drawable)
{
    W.gc_black = gdk_gc_new(drawable);
    W.gc_white = gdk_gc_new(drawable);
    W.gc_red = gdk_gc_new(drawable);

    GdkColor color = { .red = 32768, .green = 32768, .blue  = 65535 };
    gdk_gc_set_rgb_fg_color(W.gc_white, &color);
    color.green = color.blue = 0;
    gdk_gc_set_rgb_fg_color(W.gc_red, &color);
    color.red = 0;
    gdk_gc_set_rgb_fg_color(W.gc_black, &color);

}

gboolean darea_expose(GtkWidget * w, GdkEventExpose * e, gpointer user_data)
{
    (void)e; (void)user_data;

    int i;
    int j;

#if NOTMAEMO
    gdk_draw_rectangle(w->window, W.gc_black, true, 0, 0, DA_WIDTH, DA_HEIGHT);
#endif

    for (i = 0; i < 9; i++) {
	for (j = 0; j < 9; j++) {
	    int x = 4 + i * 52 + 3 * (i / 3);
	    int y = 4 + j * 52 + 3 * (j / 3);

	    gdk_draw_rgb_image(w->window, W.gc_white, x, y, 50, 50, 0,
			       tile50_pixel_data, 150);
	    //gdk_draw_rectangle(w->window, W.gc_white, true, x, y, 50, 50);
	}
    }

    for (i = 0; i < 2; i++)
	for (j = 0; j < 5; j++) {
	    int x = 64 + j * 72;
	    int y = 490 + i * 72;

	    gdk_draw_rectangle(w->window, W.gc_red, true, x, y, 64, 64);

	}
    printf("exposed\n");
    return true;
}

gboolean darea_button_press(GtkWidget * w, GdkEventButton * e, gpointer ud)
{
    (void)w; (void)ud;

    printf("buttonpress: %d %d\n", (int)e->x, (int)e->y);
    return true;
}

void darea_realize(GtkWidget * w, gpointer user_data)
{
    (void)user_data;
    drawstuff(w->window);

    gtk_widget_add_events(w, GDK_BUTTON_PRESS_MASK);

    printf("realized\n");

    gtk_signal_connect(GTK_OBJECT(w), "expose-event",
		       GTK_SIGNAL_FUNC(darea_expose), null);

    gtk_signal_connect(GTK_OBJECT(w), "button-press-event",
		       GTK_SIGNAL_FUNC(darea_button_press), null);

}



void save_and_quit(void)
{
    gtk_main_quit();
}

void buildgui(void)
{
    /* Create the main window */
#if MAEMO
    GtkWidget * mainwin = hildon_stackable_window_new();
#else
    GtkWidget * mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif

    gtk_window_set_title(GTK_WINDOW(mainwin), "Thumb Sudoku");
    g_signal_connect(G_OBJECT(mainwin), "delete_event",
                     G_CALLBACK(save_and_quit), null);

    GtkWidget * vbox = gtk_vbox_new(false, 5);
    gtk_container_add(GTK_CONTAINER(mainwin), vbox);

    GtkWidget * label = gtk_label_new("message");
    gtk_box_pack_start(GTK_BOX(vbox), label, false, false, 9);

    GtkWidget * da = gtk_drawing_area_new();
    gtk_widget_set_size_request(da, DA_WIDTH, DA_HEIGHT);
    /* needed for da->window to exist */
    gtk_widget_show(da);
    gtk_signal_connect(GTK_OBJECT(da), "realize",
		       GTK_SIGNAL_FUNC(darea_realize), null);
    gtk_box_pack_start(GTK_BOX(vbox), da, false, false, 9);

    /* Show the application window */
    gtk_widget_show_all (mainwin);

#if MAEMO
    /* portrait mode */
    portraitmode(GDK_WINDOW_XID(mainwin->window));
#endif

}


int main(int argc, char * argv[])
{
    /* Initialize i18n support */
    gtk_set_locale ();

    /* Initialize the widget set */
#if MAEMO
    hildon_gtk_init (&argc, &argv);
#else
    gtk_init(&argc, &argv);
#endif

    buildgui();

    /* Enter the main event loop, and wait for user interaction */
    gtk_main ();

    /* The user lost interest */
    return 0;
}
