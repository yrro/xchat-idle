/**
 * idle: XChat plugin that sets /away automatically.
 * Copyright Sam Morris <sam@robots.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <hexchat-plugin.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include <string.h>
#include <stdio.h>

#ifdef DEBUG
    #define debug_print(string) hexchat_print(ph, string)
    #define debug_printf(string, ...) hexchat_printf(ph, string, ...)
#else
    #define debug_print(string)
    #define debug_printf(string, ...)
#endif

/* Settings */
static const unsigned long idleThreshold = 60 * 5; // in seconds
static const char awayText[] = "Idle for >= 5 minutes.";

/* Plugin data */
static hexchat_plugin* ph;

static Display* display = NULL;
static XScreenSaverInfo* mit_info = NULL;
static int event_base, error_base;

// RFC2812 Sec 2.3: max command line length 512 characters including CRLF
#define awayCommandLength 510 + 1
static char awayCommand[awayCommandLength];

/**
 * Called every so often; if idle time > timeout, set /away.
 *
 * NB: assumes the away status is synchronised between different servers.
 */
static int checkTimeout(void* userdata __attribute__((unused))) {
    const char* awayinfo = hexchat_get_info(ph, "away");

    XScreenSaverQueryInfo(display, DefaultRootWindow(display), mit_info);

    debug_printf("Idle time = %u ms.", mit_info->idle);

    if (mit_info->idle > idleThreshold * 1000) {
        if (awayinfo == NULL) {
            debug_print("Going away!");
            hexchat_command(ph, awayCommand);
        } else {
            debug_print("already away.");
        }
    } else {
        if (awayinfo != NULL) {
            // Only set back if we set away in the first place
            if (strncmp(awayinfo, awayText, strlen(awayText)) == 0) {
                debug_print("we're back.");
                hexchat_command(ph, "allserv back");
            }
        } else {
            debug_print("still back.");
        }
    }

    return 1;
}

int hexchat_plugin_deinit(void) {
    if (mit_info) XFree(mit_info);
    if (display) XCloseDisplay(display);
    return 0;
}

int hexchat_plugin_init(
    hexchat_plugin* plugin_handle,
    const char* plugin_name[],
    const char* plugin_desc[],
    const char* plugin_version[],
    const char* arg[] __attribute__((unused))
) {
    ph = plugin_handle;

    *plugin_name = "idle";
    *plugin_desc = "sets /away automatically";
    *plugin_version = "1.0";

    if (snprintf(awayCommand, awayCommandLength, "allserv away %s", awayText) >= awayCommandLength) {
        hexchat_print(ph, "Away command too long\n");
        goto err;
    }

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        hexchat_print(ph, "XOpenDisplay failure\n");
        goto err;
    }

    if (!XScreenSaverQueryExtension(display, &event_base, &error_base)) {
        hexchat_print(ph, "XScreenSaverQueryExtension failure\n");
        goto err;
    }

    mit_info = XScreenSaverAllocInfo();
    if (!mit_info) {
        hexchat_print(ph, "XScreenSaverAllocInfo failure\n");
        goto err;
    }

    if (!hexchat_hook_timer(ph, 60 * 1000, checkTimeout, NULL)) {
        hexchat_print(ph, "hexchat_hook_timer failure\n");
        goto err;
    }

    hexchat_print(ph, "idle plugin loaded\n");
    return 1;

err:
    hexchat_plugin_deinit();
    return 0;
}

// vim: ts=8 sts=4 sw=4 et
