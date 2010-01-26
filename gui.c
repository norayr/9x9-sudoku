#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -e; TRG=`basename $0 .c`; rm -f "$TRG"
 WARN="-Wall -Wno-long-long -Wstrict-prototypes -pedantic"
 WARN="$WARN -Wcast-align -Wpointer-arith " # -Wfloat-equal #-Werror
 WARN="$WARN -W -Wwrite-strings -Wcast-qual -Wshadow" # -Wconversion
 case $1 in '') set x -O2 ### set x -ggdb;
	shift ;; esac;
 FLAGS=`pkg-config --cflags --libs hildon-1 || :`
 case $FLAGS in '') FLAGS=`pkg-config --cflags --libs gtk+-2.0`\ -DNOTMAEMO ;;
 esac
 set -x; ${CC:-gcc} -std=gnu99 $WARN "$@" -o "$TRG" "$0" $FLAGS
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
 * Last modified: Tue 26 Jan 2010 18:47:38 EET too
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

enum { false = 0, true = 1 } bool;

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

void buildgui(void)
{
    /* Create the main window */
#if MAEMO
    GtkWidget * mainwin = hildon_stackable_window_new();
#else
    GtkWidget * mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif

    GtkWidget * vbox = gtk_vbox_new(false, 5);
    gtk_container_add(mainwin, vbox);

    GtkWidget * label = gtk_label_new("message");
    gtk_box_pack_start(vbox, label, false, false, 9);

    GtkWidget * da = gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 480, 600);
    /* needed for da->window to exist */
    gtk_widget_show(da);
    gtk_box_pack_start(vbox, da, false, false, 9);

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
