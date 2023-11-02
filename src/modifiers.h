#ifndef SLACKER_MODIFIERS_H
#define SLACKER_MODIFIERS_H

#include "monitor.h"

void spawn(const Arg *arg);

void togglebar(const Arg *arg);

void toggletag(const Arg *arg);

void toggleview(const Arg *arg);

void focusstack(const Arg *arg);

void incnmaster(const Arg *arg);

void setmfact(const Arg *arg);

void zoom(const Arg *arg);

void view(const Arg *arg);

void killclient(const Arg *arg);

void setlayout(const Arg *arg);

void togglefloating(const Arg *arg);

void tag(const Arg *arg);

void focusmon(const Arg *arg);

void tagmon(const Arg *arg);

void quit(const Arg *arg);

void movemouse(const Arg *arg);

void resizemouse(const Arg *arg);

#endif // SLACKER_MODIFIERS_H
