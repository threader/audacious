/*
 * Audtool2
 * Copyright (c) 2007 Audacious development team
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <mowgli.h>
#include <locale.h>
#include "libaudclient/audctrl.h"
#include "audtool.h"

void get_current_song(gint argc, gchar **argv)
{
	gint playpos = audacious_remote_get_playlist_pos(dbus_proxy);
	gchar *song = audacious_remote_get_playlist_title(dbus_proxy, playpos);

	if (!song)
	{
		audtool_report("No song playing.");
		return;
	}

	audtool_report("%s", song);
}

void get_current_song_filename(gint argc, gchar **argv)
{
	gint playpos = audacious_remote_get_playlist_pos(dbus_proxy);
	gchar *file = audacious_remote_get_playlist_file(dbus_proxy, playpos);

	if (!file)
	{
		audtool_report("No song playing.");
		return;
	}
        
	audtool_report("%s", file);
}

void get_current_song_output_length(gint argc, gchar **argv)
{
	gint frames = audacious_remote_get_output_time(dbus_proxy);
	gint length = frames / 1000;

	audtool_report("%d:%.2d", length / 60, length % 60);
}

void get_current_song_output_length_seconds(gint argc, gchar **argv)
{
	gint frames = audacious_remote_get_output_time(dbus_proxy);
	gint length = frames / 1000;

	audtool_report("%d", length);
}

void get_current_song_output_length_frames(gint argc, gchar **argv)
{
	gint frames = audacious_remote_get_output_time(dbus_proxy);

	audtool_report("%d", frames);
}

void get_current_song_length(gint argc, gchar **argv)
{
	gint playpos = audacious_remote_get_playlist_pos(dbus_proxy);
	gint frames = audacious_remote_get_playlist_time(dbus_proxy, playpos);
	gint length = frames / 1000;

	audtool_report("%d:%.2d", length / 60, length % 60);
}

void get_current_song_length_seconds(gint argc, gchar **argv)
{
	gint playpos = audacious_remote_get_playlist_pos(dbus_proxy);
	gint frames = audacious_remote_get_playlist_time(dbus_proxy, playpos);
	gint length = frames / 1000;

	audtool_report("%d", length);
}

void get_current_song_length_frames(gint argc, gchar **argv)
{
	gint playpos = audacious_remote_get_playlist_pos(dbus_proxy);
	gint frames = audacious_remote_get_playlist_time(dbus_proxy, playpos);

	audtool_report("%d", frames);
}

void get_current_song_bitrate(gint argc, gchar **argv)
{
	gint rate, freq, nch;

	audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);

	audtool_report("%d", rate);
}

void get_current_song_bitrate_kbps(gint argc, gchar **argv)
{
	gint rate, freq, nch;

	audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);

	audtool_report("%d", rate / 1000);
}

void get_current_song_frequency(gint argc, gchar **argv)
{
	gint rate, freq, nch;

	audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);

	audtool_report("%d", freq);
}

void get_current_song_frequency_khz(gint argc, gchar **argv)
{
	gint rate, freq, nch;

	audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);

	audtool_report("%0.1f", (gfloat) freq / 1000);
}

void get_current_song_channels(gint argc, gchar **argv)
{
	gint rate, freq, nch;

	audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);

	audtool_report("%d", nch);
}

void get_current_song_tuple_field_data(gint argc, gchar **argv)
{
	gchar *data;

	if (argc < 2)
	{
		audtool_whine_args(argv[0], "<fieldname>");
		audtool_whine_tuple_fields();
		exit(1);
	}

	if (!(data = audacious_get_tuple_field_data(dbus_proxy, argv[1], audacious_remote_get_playlist_pos(dbus_proxy))))
	{
		return;
	}
	
	audtool_report("%s", data);

	g_free(data);
}

void get_current_song_info(gint argc, gchar **argv)
{
    gint rate, freq, nch;

    audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);
    audtool_report("rate = %d freq = %d nch = %d", rate, freq, nch);
}

