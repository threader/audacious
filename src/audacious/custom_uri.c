/*
 * Audacious
 * Copyright (c) 2007 William Pitcock
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses>.
 *
 * The Audacious team does not consider modular code linking to
 * Audacious or using our public API to be a derived work.
 */

#include "custom_uri.h"

mowgli_dictionary_t *uri_type_dict = NULL;

void uri_set_plugin(const gchar *uri, InputPlugin *ip)
{
    g_return_if_fail(uri != NULL);
    g_return_if_fail(ip != NULL);

    if (uri_type_dict == NULL)
        uri_type_dict = mowgli_dictionary_create(strcasecmp);
    else if (mowgli_dictionary_find(uri_type_dict, uri))
        mowgli_dictionary_delete(uri_type_dict, uri);
    mowgli_dictionary_add(uri_type_dict, uri, ip);
}

InputPlugin *uri_get_plugin(const gchar *filename)
{
    gchar *uri, *pos;
    InputPlugin *ip;

    if (filename == NULL)
        return NULL;

    if (uri_type_dict == NULL)
        return NULL;

    pos = strstr(filename, "://");
    if (pos)
        uri = g_strndup(filename, pos - filename + 3);
    else
        return NULL;

    ip = mowgli_dictionary_retrieve(uri_type_dict, uri);
 
    g_free(uri);
    
    return ip;
}
