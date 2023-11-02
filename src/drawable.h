#ifndef SLACKER_DRAWABLE_H
#define SLACKER_DRAWABLE_H

// X11 Libraries
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

// Standard Libraries

typedef struct SlackerCursor SlackerCursor;
struct SlackerCursor {
	Cursor cursor;
};

typedef struct Fnt Fnt;
struct Fnt {
	Display *dpy;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
	Fnt *next;
};

enum ColorSchemeIndex { ColFg, ColBg, ColBorder };

typedef XftColor SlackerColor;

typedef struct Drw Drw;
struct Drw {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	SlackerColor *scheme;
	Fnt *fonts;
};

/* Drawable abstraction */
Drw *drw_create(Display *dpy, int screen, Window win, unsigned int w,
		unsigned int h);
void drw_resize(Drw *drw, unsigned int w, unsigned int h);
void drw_free(Drw *drw);

/* Fnt abstraction */
Fnt *drw_fontset_create(Drw *drw, const char *font);
void drw_fontset_free(Fnt *set);
unsigned int drw_fontset_getwidth(Drw *drw, const char *text);
unsigned int drw_fontset_getwidth_clamp(Drw *drw, const char *text,
					unsigned int n);
void drw_font_getexts(Fnt *font, const char *text, unsigned int len,
		      unsigned int *w, unsigned int *h);

/* Colorscheme abstraction */
void drw_clr_create(Drw *drw, SlackerColor *dest, const char *clrname);
SlackerColor *drw_scm_create(Drw *drw, const char *clrnames[], size_t clrcount);

/* Cursor abstraction */
SlackerCursor *drw_cur_create(Drw *drw, int shape);
void drw_cur_free(Drw *drw, SlackerCursor *cursor);

/* Drawing context manipulation */
void drw_setfontset(Drw *drw, Fnt *set);
void drw_setscheme(Drw *drw, SlackerColor *scm);

/* Drawing functions */
void drw_rect(Drw *drw, int x, int y, unsigned int w, unsigned int h,
	      int filled, int invert);
int drw_text(Drw *drw, int x, int y, unsigned int w, unsigned int h,
	     unsigned int lpad, const char *text, int invert);

/* Map functions */
void drw_map(Drw *drw, Window win, int x, int y, unsigned int w,
	     unsigned int h);

#endif // SLACKER_DRAW_H
