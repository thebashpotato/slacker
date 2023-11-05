#ifndef SLACKER_MODIFIERS_H
#define SLACKER_MODIFIERS_H

#include "monitor.h"

void Slacker__spawn(const Arg *arg);

void Slacker__togglebar(const Arg *arg);

void Slacker__toggletag(const Arg *arg);

void Slacker__toggleview(const Arg *arg);

void Slacker__focus_stack(const Arg *arg);

void Slacker__increment_n_master(const Arg *arg);

void Slacker__setmfact(const Arg *arg);

void Slacker__zoom(const Arg *arg);

void Slacker__view(const Arg *arg);

void Slacker__kill_client(const Arg *arg);

void Slacker__setlayout(const Arg *arg);

void Slacker__togglefloating(const Arg *arg);

void Slacker__tag(const Arg *arg);

void Slacker__focus_monitor(const Arg *arg);

void Slacker__tagmon(const Arg *arg);

void Slacker__quit(const Arg *arg);

void Slacker__move_with_mouse(const Arg *arg);

void Slacker__resize_client_with_mouse(const Arg *arg);

#endif // SLACKER_MODIFIERS_H
