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

#ifndef AUDACIOUS_EQUALIZER_H
#define AUDACIOUS_EQUALIZER_H

#include <glib.h>

void init_equalizer (void);

gfloat equalizer_get_preamp(void);
void equalizer_set_preamp(gfloat preamp);
gfloat equalizer_get_band(gint band);
void equalizer_set_band(gint band, gfloat value);

gboolean equalizer_get_active();
void equalizer_set_active(gboolean active);

#endif
