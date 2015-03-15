/**
 * idle: XChat plugin that sets /away automatically.
 * Copyright 2004-2007 Sam Morris <sam@robots.org.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <xchat/xchat-plugin.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include <string.h>
#include <stdio.h>

#ifdef DEBUG
    #define debug_print(string) xchat_print(ph, string)
    #define debug_printf(string, ...) xchat_printf(ph, string, ...)
#else
    #define debug_print(string)
    #define debug_printf(string, ...)
#endif

/* Settings */
static const unsigned long idleThreshold = 60 * 5; // in seconds
static const char* awayText = "Idle for >= 5 minutes.";

/* Plugin data */
static xchat_plugin* ph;

static Display* display = NULL;
static Window window = 0;
static XScreenSaverInfo* mit_info;
static int event_base, error_base;

// RFC2812 Sec 2.3: max command line length 512 characters including CRLF
#define awayCommandLength 510
//static const int awayCommandLength = 510; // CRAPPY C, GRR

static char awayCommand[awayCommandLength];

/**
 * Sends a command to every server that we are connected to.
 */
static void send_command(xchat_plugin* ph, const char* command) {
    xchat_command(ph, command);
    debug_print("command sent");
}

/**
 * Called every so often; if idle time > timeout, set /away.
 *
 * NB: assumes the away status is synchronised between different servers.
 */
static int checkTimeout(void* userdata) {
    const char* awayinfo = xchat_get_info(ph, "away");

    XScreenSaverQueryInfo(display, window, mit_info);

    debug_printf("Idle time = %u ms.", mit_info->idle);

    if (mit_info->idle > idleThreshold * 1000) {
        if (awayinfo == NULL) {
            debug_print("Going away!");
            send_command(ph, awayCommand);
        } else {
            debug_print("already away.");
        }
    } else {
        if (awayinfo != NULL) {
            // Only set back if we set away in the first place
            if (strncmp(awayinfo, awayText, strlen(awayText)) == 0) {
                debug_print("we're back.");
                send_command(ph, "allserv back");
            }
        } else {
            debug_print("still back.");
        }
    }

    return 1;
}

int xchat_plugin_deinit(void) {
    XFree(mit_info);
}

int xchat_plugin_init(
    xchat_plugin* plugin_handle,
    char** plugin_name,
    char** plugin_desc,
    char** plugin_version,
    char** arg
) {
    ph = plugin_handle;

    *plugin_name = "idle";
    *plugin_desc = "sets /away automatically";
    *plugin_version = "0.1";

    snprintf(awayCommand, awayCommandLength, "allserv away %s", awayText);

    display = XOpenDisplay(NULL);
    if (display == NULL)
        return 0;

    window = DefaultRootWindow(display);
    if (window == 0)
        return 0;

    if (!XScreenSaverQueryExtension(display, &event_base, &error_base))
        return 0;

    mit_info = XScreenSaverAllocInfo();

    xchat_hook_timer(ph, 10 * 1000, checkTimeout, NULL);

    xchat_print(ph, "idle plugin loaded.");

    return 1;
}

// vim: ts=8 sts=4 sw=4 et
