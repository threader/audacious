/*
 * scanner.c
 * Copyright 2009 John Lindgren
 *
 * This file is part of Audacious.
 *
 * Audacious is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License.
 *
 * Audacious is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Audacious. If not, see <http://www.gnu.org/licenses/>.
 *
 * The Audacious team does not consider modular code linking to Audacious or
 * using our public API to be a derived work.
 */

#include <glib.h>

#include "playlist.h"

/*
 * Note: There may be a more efficient way to do this. We keep the playlist
 * locked almost constantly and iterate through it each cycle. If you change the
 * code, though, be careful not to create any race conditions. -jlindgren
 */

static GMutex * mutex;
static GCond * wake;
static GThread * thread;
static Playlist * active;
static gboolean enabled, reset, quit;

static void * scanner (void * unused)
{
    gboolean done = FALSE;
    GList * node;
    PlaylistEntry * entry;

    while (TRUE)
    {
        g_mutex_lock (mutex);

        if (quit)
        {
            g_mutex_unlock (mutex);
            break;
        }

        if (reset)
        {
            done = FALSE;
            reset = FALSE;
        }

        if (active == NULL)
            done = TRUE;

        if (! enabled || done)
        {
            g_cond_wait (wake, mutex);
            g_mutex_unlock (mutex);
            continue;
        }

        PLAYLIST_LOCK (active);

        for (node = active->entries; node != NULL; node = node->next)
        {
            entry = node->data;

            if (entry->tuple == NULL && ! entry->failed)
                goto FOUND;
        }

        PLAYLIST_UNLOCK (active);
        g_mutex_unlock (mutex);

        done = TRUE;
        continue;

      FOUND:
        playlist_entry_get_info (entry);

        PLAYLIST_UNLOCK (active);
        g_mutex_unlock (mutex);

        event_queue ("playlist update", NULL);
    }

    return NULL;
}

void scanner_init (void)
{
    mutex = g_mutex_new ();
    wake = g_cond_new ();

    active = NULL;
    enabled = FALSE;
    reset = FALSE;
    quit = FALSE;

    thread = g_thread_create (scanner, NULL, TRUE, NULL);
}

void scanner_enable (gboolean enable)
{
    g_mutex_lock (mutex);
    enabled = enable;
    g_cond_signal (wake);
    g_mutex_unlock (mutex);
}

void scanner_reset (void)
{
    g_mutex_lock (mutex);
    reset = TRUE;
    g_cond_signal (wake);
    g_mutex_unlock (mutex);
}

void scanner_end (void)
{
    g_mutex_lock (mutex);
    quit = TRUE;
    g_cond_signal (wake);
    g_mutex_unlock (mutex);

    g_thread_join (thread);

    g_mutex_free (mutex);
    g_cond_free (wake);
}

Playlist * get_active_playlist (void)
{
    return active;
}

void set_active_playlist (Playlist * playlist)
{
    if (playlist == active)
        return;

    g_mutex_lock (mutex);
    active = playlist;
    reset = TRUE;
    g_cond_signal (wake);
    g_mutex_unlock (mutex);
}
