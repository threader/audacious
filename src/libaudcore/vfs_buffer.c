/*  Audacious
 *  Copyright (c) 2006-2007 William Pitcock
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

#include <glib.h>
#include <string.h>
#include "vfs.h"
#include "vfs_buffer.h"

VFSFile *
buffer_vfs_fopen_impl(const gchar * path,
          const gchar * mode)
{
    return NULL;
}

gint
buffer_vfs_fclose_impl(VFSFile * file)
{
    g_return_val_if_fail(file != NULL, -1);

    if (file->handle)
        g_free(file->handle);

    return 0;
}

size_t
buffer_vfs_fread_impl(gpointer i_ptr,
          size_t size,
          size_t nmemb,
          VFSFile * file)
{
    VFSBuffer *handle;
    guchar *i;
    size_t read = 0;
    guchar *ptr = (guchar *) i_ptr;

    if (file == NULL)
        return 0;

    handle = (VFSBuffer *) file->handle;

    for (i = ptr; (gsize) (i - ptr) < nmemb * size && 
	 (gsize) (i - ptr) <= handle->size;
	 i++, handle->iter++)
    {
       *i = *handle->iter;
       read++;
    }

    return (read / size);
}

size_t
buffer_vfs_fwrite_impl(gconstpointer i_ptr,
           size_t size,
           size_t nmemb,
           VFSFile * file)
{
    VFSBuffer *handle;
    const guchar *i;
    size_t written = 0;
    const guchar *ptr = (const guchar *) i_ptr;

    if (file == NULL)
        return 0;

    handle = (VFSBuffer *) file->handle;

    for (i = ptr; (gsize) (i - ptr) < nmemb * size &&
	 (gsize) (i - ptr) <= handle->size;
	 i++, handle->iter++)
    {
       *handle->iter = *i;
       written++;
    }

    return (written / size);
}

gint
buffer_vfs_getc_impl(VFSFile *stream)
{
  VFSBuffer *handle = (VFSBuffer *) stream->handle;
  gint c;

  c = *handle->iter;
  handle->iter++;

  return c;
}

gint
buffer_vfs_ungetc_impl(gint c, VFSFile *stream)
{
  return -1;
}

gint
buffer_vfs_fseek_impl(VFSFile * file,
          glong offset,
          gint whence)
{
    VFSBuffer *handle;

    if (file == NULL)
        return 0;

    handle = (VFSBuffer *) file->handle;

    switch(whence)
    {
       case SEEK_CUR:
          handle->iter = handle->iter + offset;
          break;
       case SEEK_END:
          handle->iter = handle->end;
          break;
       case SEEK_SET:
       default:
          handle->iter = handle->data + offset;
          break;
    }

    return 0;
}

void
buffer_vfs_rewind_impl(VFSFile * file)
{
    VFSBuffer *handle;

    if (file == NULL)
        return;

    handle = (VFSBuffer *) file->handle;

    handle->iter = handle->data;
}

glong
buffer_vfs_ftell_impl(VFSFile * file)
{
    VFSBuffer *handle;

    if (file == NULL)
        return 0;

    handle = (VFSBuffer *) file->handle;

    return handle->iter - handle->data;
}

gboolean
buffer_vfs_feof_impl(VFSFile * file)
{
    VFSBuffer *handle;

    if (file == NULL)
        return FALSE;

    handle = (VFSBuffer *) file->handle;

    return (gboolean) (handle->iter == handle->end);
}

gint
buffer_vfs_truncate_impl(VFSFile * file, glong size)
{
    return 0;
}

off_t
buffer_vfs_fsize_impl(VFSFile * file)
{
    VFSBuffer *handle;

    if (file == NULL)
        return -1;

    handle = (VFSBuffer *) file->handle;

    return (off_t)handle->end;
}

VFSConstructor buffer_const = {
	NULL,			// not a normal VFS class
	buffer_vfs_fopen_impl,
	buffer_vfs_fclose_impl,
	buffer_vfs_fread_impl,
	buffer_vfs_fwrite_impl,
	buffer_vfs_getc_impl,
	buffer_vfs_ungetc_impl,
	buffer_vfs_fseek_impl,
	buffer_vfs_rewind_impl,
	buffer_vfs_ftell_impl,
	buffer_vfs_feof_impl,
	buffer_vfs_truncate_impl,
	buffer_vfs_fsize_impl,
	NULL
};

VFSFile *
vfs_buffer_new(gpointer data, gsize size)
{
    VFSFile *handle;
    VFSBuffer *buffer;

    g_return_val_if_fail(data != NULL, NULL);
    g_return_val_if_fail(size > 0, NULL);

    handle = g_new0(VFSFile, 1);

    buffer = g_new0(VFSBuffer, 1);
    buffer->data = data;
    buffer->iter = data;
    buffer->end = (guchar *) data + size;
    buffer->size = size;

    handle->handle = buffer;
    handle->base = &buffer_const;
    handle->ref = 1;

    return handle;
}

VFSFile *
vfs_buffer_new_from_string(gchar *str)
{
    g_return_val_if_fail(str != NULL, NULL);

    return vfs_buffer_new(str, strlen(str));
}
