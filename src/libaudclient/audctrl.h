/*
 * Audacious: A cross-platform multimedia player
 * Copyright (c) 2007 Ben Tucker
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; under version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef AUDACIOUS_AUDCTRL_H
#define AUDACIOUS_AUDCTRL_H

#include <glib.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

    void audacious_remote_playlist(DBusGProxy *proxy, gchar **list, gint num,
                                   gboolean enqueue);
    gchar *audacious_remote_get_version(DBusGProxy *proxy);
    void audacious_remote_playlist_add(DBusGProxy *proxy, GList *list);
    void audacious_remote_playlist_delete(DBusGProxy *proxy, guint pos);
    void audacious_remote_play(DBusGProxy *proxy);
    void audacious_remote_pause(DBusGProxy *proxy);
    void audacious_remote_stop(DBusGProxy *proxy);
    gboolean audacious_remote_is_playing(DBusGProxy *proxy);
    gboolean audacious_remote_is_paused(DBusGProxy *proxy);
    gint audacious_remote_get_playlist_pos(DBusGProxy *proxy);
    void audacious_remote_set_playlist_pos(DBusGProxy *proxy, guint pos);
    gint audacious_remote_get_playlist_length(DBusGProxy *proxy);
    void audacious_remote_playlist_clear(DBusGProxy *proxy);
    gint audacious_remote_get_output_time(DBusGProxy *proxy);
    void audacious_remote_jump_to_time(DBusGProxy *proxy, guint pos);
    void audacious_remote_get_volume(DBusGProxy *proxy, gint *vl, gint *vr);
    gint audacious_remote_get_main_volume(DBusGProxy *proxy);
    gint audacious_remote_get_balance(DBusGProxy *proxy);
    void audacious_remote_set_volume(DBusGProxy *proxy, gint vl, gint vr);
    void audacious_remote_set_main_volume(DBusGProxy *proxy, gint v);
    void audacious_remote_set_balance(DBusGProxy *proxy, gint b);
    gchar *audacious_remote_get_skin(DBusGProxy *proxy);
    void audacious_remote_set_skin(DBusGProxy *proxy, gchar *skinfile);
    gchar *audacious_remote_get_playlist_file(DBusGProxy *proxy, guint pos);
    gchar *audacious_remote_get_playlist_title(DBusGProxy *proxy, guint pos);
    gint audacious_remote_get_playlist_time(DBusGProxy *proxy, guint pos);
    void audacious_remote_get_info(DBusGProxy *proxy, gint *rate, gint *freq,
                                   gint *nch);
    void audacious_remote_main_win_toggle(DBusGProxy *proxy, gboolean show);
    void audacious_remote_pl_win_toggle(DBusGProxy *proxy, gboolean show);
    void audacious_remote_eq_win_toggle(DBusGProxy *proxy, gboolean show);
    gboolean audacious_remote_is_main_win(DBusGProxy *proxy);
    gboolean audacious_remote_is_pl_win(DBusGProxy *proxy);
    gboolean audacious_remote_is_eq_win(DBusGProxy *proxy);
    void audacious_remote_show_prefs_box(DBusGProxy *proxy);
    void audacious_remote_toggle_aot(DBusGProxy *proxy, gboolean ontop);
    void audacious_remote_eject(DBusGProxy *proxy);
    void audacious_remote_playlist_prev(DBusGProxy *proxy);
    void audacious_remote_playlist_next(DBusGProxy *proxy);
    void audacious_remote_playlist_add_url_string(DBusGProxy *proxy,
                                                  gchar *string);
    gboolean audacious_remote_is_running(DBusGProxy *proxy);
    void audacious_remote_toggle_repeat(DBusGProxy *proxy);
    void audacious_remote_toggle_shuffle(DBusGProxy *proxy);
    gboolean audacious_remote_is_repeat(DBusGProxy *proxy);
    gboolean audacious_remote_is_shuffle(DBusGProxy *proxy);
    void audacious_remote_get_eq(DBusGProxy *proxy, gdouble *preamp,
                                 GArray **bands);
    gdouble audacious_remote_get_eq_preamp(DBusGProxy *proxy);
    gdouble audacious_remote_get_eq_band(DBusGProxy *proxy, gint band);
    void audacious_remote_set_eq(DBusGProxy *proxy, gdouble preamp,
                                 GArray *bands);
    void audacious_remote_set_eq_preamp(DBusGProxy *proxy, gdouble preamp);
    void audacious_remote_set_eq_band(DBusGProxy *proxy, gint band,
                                      gdouble value);

/* Added in XMMS 1.2.1 */
    void audacious_remote_quit(DBusGProxy *proxy);

/* Added in XMMS 1.2.6 */
    void audacious_remote_play_pause(DBusGProxy *proxy);
    void audacious_remote_playlist_ins_url_string(DBusGProxy *proxy,
                                                  gchar *string, guint pos);

/* Added in XMMS 1.2.11 */
    void audacious_remote_playqueue_add(DBusGProxy *proxy, guint pos);
    void audacious_remote_playqueue_remove(DBusGProxy *proxy, guint pos);
    gint audacious_remote_get_playqueue_length(DBusGProxy *proxy);
    void audacious_remote_toggle_advance(DBusGProxy *proxy);
    gboolean audacious_remote_is_advance(DBusGProxy *proxy);

/* Added in BMP 0.9.7 */
    void audacious_remote_activate(DBusGProxy *proxy);

/* Added in Audacious 1.1 */
    void audacious_remote_show_jtf_box(DBusGProxy *proxy);
    void audacious_remote_playqueue_clear(DBusGProxy *proxy);
    gboolean audacious_remote_playqueue_is_queued(DBusGProxy *proxy, guint pos);
    gint audacious_remote_get_playqueue_list_position(DBusGProxy *proxy, guint qpos);
    gint audacious_remote_get_playqueue_queue_position(DBusGProxy *proxy, guint pos);

/* Added in Audacious 1.2 */
    void audacious_set_session_uri(DBusGProxy *proxy, gchar *uri);
    gchar *audacious_get_session_uri(DBusGProxy *proxy);
    void audacious_set_session_type(DBusGProxy *proxy, gint type);

/* Added in Audacious 1.3 */
    void audacious_remote_playlist_enqueue_to_temp(DBusGProxy *proxy,
                                                   gchar *string);
    gchar *audacious_get_tuple_field_data(DBusGProxy *proxy, gchar *field,
                                          guint pos);
/* Added in Audacious 1.4 */
    void audacious_remote_show_about_box(DBusGProxy *proxy);
    void audacious_remote_toggle_about_box(DBusGProxy *proxy, gboolean show);
    void audacious_remote_toggle_jtf_box(DBusGProxy *proxy, gboolean show);
    void audacious_remote_toggle_prefs_box(DBusGProxy *proxy, gboolean show);
    void audacious_remote_toggle_filebrowser(DBusGProxy *proxy, gboolean show);
    void audacious_remote_eq_activate(DBusGProxy *proxy, gboolean active);

/* Added in Audacious 1.9 */
    gchar **audacious_remote_get_tuple_fields(DBusGProxy *proxy);
G_END_DECLS

#endif /* AUDACIOUS_AUDCTRL_H */
