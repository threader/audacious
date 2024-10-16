/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2008  Audacious development team.
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


#include "icons-stock.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>


static void
load_stock_icon(gchar *id, gchar *filename, GtkIconFactory *iconfactory)
{
    GtkIconSet *iconset;
    GdkPixbuf *pixbuf;

    pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if (pixbuf == NULL)
        return;

    iconset = gtk_icon_set_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    gtk_icon_factory_add(iconfactory, id, iconset);
}

void
register_aud_stock_icons(void)
{
    GtkIconFactory *iconfactory = gtk_icon_factory_new();

    load_stock_icon(AUD_STOCK_PLAYLIST,
                    DATA_DIR "/images/menu_playlist.png", iconfactory);
    load_stock_icon(AUD_STOCK_PLUGIN,
                    DATA_DIR "/images/menu_plugin.png", iconfactory);
    load_stock_icon(AUD_STOCK_QUEUETOGGLE,
                    DATA_DIR "/images/menu_queue_toggle.png", iconfactory);
    load_stock_icon(AUD_STOCK_RANDOMIZEPL,
                    DATA_DIR "/images/menu_randomize_playlist.png", iconfactory);
    

    gtk_icon_factory_add_default( iconfactory );
    g_object_unref( iconfactory );
}

