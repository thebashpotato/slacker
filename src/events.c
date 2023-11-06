#include <stdio.h>
#include "events.h"

void Slacker__event_loop(XEvent *event)
{
	switch (event->type) {
	case ButtonPress:
		event_buttonpress(event);
		break;
	case ClientMessage:
		event_clientmessage(event);
		break;
	case ConfigureRequest:
		event_configurerequest(event);
		break;
	case ConfigureNotify:
		event_configurenotify(event);
		break;
	case DestroyNotify:
		event_destroynotify(event);
		break;
	case EnterNotify:
		event_enternotify(event);
		break;
	case Expose:
		event_expose(event);
		break;
	case FocusIn:
		event_focusin(event);
		break;
	case KeyPress:
		event_keypress(event);
		break;
	case MappingNotify:
		event_mappingnotify(event);
		break;
	case MapRequest:
		event_maprequest(event);
		break;
	case MotionNotify:
		event_motionnotify(event);
		break;
	case PropertyNotify:
		event_propertynotify(event);
		break;
	case UnmapNotify:
		event_unmapnotify(event);
		break;
	default:
		if (DEBUG == 1) {
			fprintf(stdout, "Unhandled event: %d\n", event->type);
		}
		break;
	}
}
