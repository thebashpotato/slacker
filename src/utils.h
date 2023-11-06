#ifndef SWM_UTILS_H
#define SWM_UTILS_H

#include <stdlib.h>

/// @brief Generic purpose macros and helper functions

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))
#define LENGTH(X) (sizeof X / sizeof X[0])

//////////////////////////////////////////
/// Window Mangager specific macros
//////////////////////////////////////////

/// Client text which displays in the bar when the client is broken
// TODO: Move this into the Client struct
extern const char CLIENT_WINDOW_BROKEN[];

#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)

#define CLEANMASK(mask)                                              \
	(mask & ~(g_slacker.numlockmask | LockMask) &                \
	 (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | \
	  Mod4Mask | Mod5Mask))

#define INTERSECT(x, y, w, h, m)                                         \
	(MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) * \
	 MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))

#define ISVISIBLE(Client) \
	((Client->tags & Client->mon->tagset[Client->mon->seltags]))

#define MOUSEMASK (BUTTONMASK | PointerMotionMask)

#define WIDTH(X) ((X)->w + 2 * (X)->bw)

#define HEIGHT(X) ((X)->h + 2 * (X)->bw)

#define TAGMASK ((1 << LENGTH(G_TAGS)) - 1)

#define TEXTW(X)                                     \
	(drw_fontset_getwidth(g_slacker.draw, (X)) + \
	 g_slacker.left_right_padding_sum + 2)

/// @brief Print error message and exit the window manager
void die(const char *fmt, ...);

/// @brief Allocate memory and check for errors
void *ecalloc(size_t nmemb, size_t size);

#endif // SLACKER_UTILS_H
