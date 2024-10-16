/*  Audacious
 *  Copyright (C) 2005-2007  Audacious development team.
 *
 *  BMP - Cross-platform multimedia player
 *  Copyright (C) 2003-2004  BMP development team.
 *
 *  Based on XMMS:
 *  Copyright (C) 1998-2003  XMMS development team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; under version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses>.
 *
 *  The Audacious team does not consider modular code linking to
 *  Audacious or using our public API to be a derived work.
 */

#ifndef AUDACIOUS_STRINGS_H
#define AUDACIOUS_STRINGS_H

#include <stdlib.h>
#include <glib.h>

G_BEGIN_DECLS

gchar *escape_shell_chars(const gchar * string);

gchar *str_append(gchar * str, const gchar * add_str);
gchar *str_replace(gchar * str, gchar * new_str);
void str_replace_in(gchar ** str, gchar * new_str);

gboolean str_has_prefix_nocase(const gchar * str, const gchar * prefix);
gboolean str_has_suffix_nocase(const gchar * str, const gchar * suffix);
gboolean str_has_suffixes_nocase(const gchar * str, gchar * const *suffixes);

gchar *str_assert_utf8(const gchar *str);
extern gchar *(*str_to_utf8)(const gchar * str);
gchar *str_to_utf8_fallback(const gchar * str);

gchar *filename_to_utf8(const gchar * filename);
gchar *uri_to_display_basename(const gchar * uri);
gchar *uri_to_display_dirname(const gchar * uri);

const gchar *str_skip_chars(const gchar * str, const gchar * chars);

gchar *convert_title_text(gchar * text);
gchar *convert_dos_path(gchar * text);

extern gchar *(*chardet_to_utf8)(const gchar *str, gssize len,
                       gsize *arg_bytes_read, gsize *arg_bytes_write,
                       GError **arg_error);

G_END_DECLS

#endif /* AUDACIOUS_STRINGS_H */
