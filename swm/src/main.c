/// # Slacker Window Manager (swm) is a suckless-style X window manager.
///
/// ## Introduction
///
/// 	Swm is an attempt at creating a more sound and concise version of dwm
/// 	using straight forward C design patterns, refactoring as much spaghetti code as possible, and using explicit naming conventions.
///		Swm does not read a configuration file, but does rely on a configuration library where global variables can be updated.
/// 	As far as the user is concerned they will still edit global variable values in a C config, its just in a different folder.
/// 	The shared library approach will offer better code decoupling than the suckless approach, and will allow me to port other tools
///		like dmenu, st, slock etc, as their configuration variables can be added to this single source of config truth `libslacker_config.so`.
///		This also makes hot-reloading of configuration possible.
///
/// ## Goals
///
/// 1.	Restructure and clean up the entire code base to make it more maintainable,
///		 	and easier to expand upon while also easier to read/understand and contribute to for (new comers).
/// 	 	This means the suckless patching approach will not work, since the code-base is so different.
///		 	Because of this, swm will provide a more feature rich and batteries-included
///		 	experience out of the box. These features will be included but not limited to:
///
///		- Sys tray support
///		- Dwm blocks like support (rather baked into the code and not a separate program)
/// 	- Window gaps support
/// 	- Vertically resizing windows
/// 	- Swap any non-master client window, with the master window in the stack.
/// 	- More will be added as I think of them, or as other people request.
///
/// 2.	There will be no arbitary limitation on the source lines of code (SLOC) like dwm has or had.
///			The project will get as big as it needs to be to provide a good user experience and at the very least the features I want.
///			The executable size is 67K, I mean come on, if you think that is outrageous, you need to give your balls a tug.
///
/// 3. 	There will be a nice bloated javascript companion documentation site, where the data structures, control flow
///			will be defined and explained. I will also be adding tutorials on how to add common dwm patches to swm, that it does not have out of the box.
///			This is in an effort to help people learn C, client side X11 programming and to "demystify" the software development process as it
/// 		relates to the latter and suckless-style software in general.
///
/// 4. 	The project does not aim to be "small and elitist" (although it will likely stay small because X11 is dead),
///			and doesn't care about stupid questions. Stupid questions exist, because we all start somewhere.

// Standard Libraries
#include <locale.h>
#include <stdio.h>
#include <unistd.h>

// Slacker Headers
#include "autostart.h"
#include "utils.h"
#include "swm.h"

int main(int argc, char **argv)
{
	if (DEBUG == true) {
		printf("Running in debug mode, attach debugger to pid: '%d'\n",
		       getpid());
		//sleep(15);
	}
	if (argc == 2 && !strcmp("-v", argv[1])) {
		fprintf(stdout, "swm-" VERSION "\n");
		return EXIT_SUCCESS;
	} else {
		if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
			fputs("warning: no locale support\n", stderr);
		}

		clean_environment();
		Autostart as = Autostart__new();

		if (DEBUG == false) {
			Autostart__add(&as, "xset r rate 200 60");
			Autostart__add(&as, "setxkbmap -option ctrl:nocaps");
			Autostart__add(&as, "picom");
			Autostart__add(
				&as,
				"feh --bg-fill /usr/local/share/slacker/background.jpg");
			Autostart__add(
				&as,
				"xrandr --output DisplayPort-0 --mode 3840x1080 --rate 143.85");

			Autostart__exec(&as);
		}

		Swm__startup();
		Swm__run();
		Swm__delete();
		Autostart__kill(&as);
		clean_environment();
	}

	return EXIT_SUCCESS;
}
