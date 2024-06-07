// Standard Libraries
#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Slacker Headers
#include "autostart.h"
#include "utils.h"
#include "swm.h"

int main(int argc, char **argv)
{
#if (DEBUG == true)
	printf("Running in debug mode, attach debugger to pid: '%d'\n",
	       getpid());
	sleep(15);
#endif

	if (argc == 2 && !strcmp("-v", argv[1])) {
		fprintf(stdout, "swm-" VERSION "\n");
		return EXIT_SUCCESS;
	} else {
		if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
			fputs("warning: no locale support\n", stderr);
		}
		clean_environment();
		Autostart as = Autostart__new();

#if (DEBUG == false)
		printf("Running in release mode\n");
		Autostart__add(&as, "xset r rate 200 60");
		Autostart__add(&as, "setxkbmap -option ctrl:nocaps");
		Autostart__add(&as, "picom");
		Autostart__add(&as, "slacker_update_bar.sh");
		Autostart__add(
			&as,
			"feh --bg-fill /usr/local/share/slacker/background.png");

		// NOTE: Example for ultra wide monitor on Display port (GPU) with 144hz refresh rate
		Autostart__add(
			&as,
			"xrandr --output DP-3 --mode 3840x1080 --rate 143.72");

		// NOTE: Example for built-in laptop screen for Dell xps 13
		// Autostart__add(
		// &as,
		// "xrandr --output eDP-1 --mode 1920x1200 --rate 59.95");

		Autostart__exec(&as);
#endif

		Swm__startup();
		Swm__run();
		Swm__delete();
		Autostart__kill(&as);
		clean_environment();
	}

	return EXIT_SUCCESS;
}
