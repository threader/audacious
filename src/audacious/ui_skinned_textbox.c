/*
 * Audacious - a cross-platform multimedia player
 * Copyright (c) 2007  Audacious development team.
 *
 * Based on:
 * BMP - Cross-platform multimedia player
 * Copyright (C) 2003-2004  BMP development team.
 * XMMS:
 * Copyright (C) 1998-2003  XMMS development team.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "widgets/widgetcore.h"
#include "ui_skinned_textbox.h"
#include "main.h"
#include "util.h"
#include "strings.h"
#include <string.h>
#include <ctype.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkimage.h>

#define UI_SKINNED_TEXTBOX_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), UI_TYPE_SKINNED_TEXTBOX, UiSkinnedTextboxPrivate))
typedef struct _UiSkinnedTextboxPrivate UiSkinnedTextboxPrivate;

static GMutex *mutex = NULL;

enum {
    CLICKED,
    DOUBLE_CLICKED,
    RIGHT_CLICKED,
    DOUBLED,
    REDRAW,
    LAST_SIGNAL
};

struct _UiSkinnedTextboxPrivate {
    GtkWidget        *image;
    GdkGC            *gc;
    SkinPixmapId     skin_index;
    GtkWidget        *fixed;
    gboolean         double_size;
    gboolean         scroll_back;
    gint             nominal_y, nominal_height;
    gint             scroll_timeout;
    gint             font_ascent, font_descent;
    PangoFontDescription *font;
    gchar            *fontname;
    gchar            *pixmap_text;
    gint             skin_id;
    gint             drag_x, drag_off, offset;
    gboolean         is_scrollable, is_dragging;
    gint             pixmap_width;
    GdkPixmap        *pixmap;
    gboolean         scroll_allowed, scroll_enabled;
    gint             scroll_dummy;
    gint             resize_width, resize_height;
    gint             move_x, move_y;
};


static GtkBinClass *parent_class = NULL;
static void ui_skinned_textbox_class_init(UiSkinnedTextboxClass *klass);
static void ui_skinned_textbox_init(UiSkinnedTextbox *textbox);
static GObject* ui_skinned_textbox_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_params);
static void ui_skinned_textbox_destroy(GtkObject *object);
static void ui_skinned_textbox_realize(GtkWidget *widget);
static void ui_skinned_textbox_unrealize(GtkWidget *widget);
static void ui_skinned_textbox_map(GtkWidget *widget);
static void ui_skinned_textbox_unmap(GtkWidget *widget);
static gint ui_skinned_textbox_expose(GtkWidget *widget,GdkEventExpose *event);

static void ui_skinned_textbox_size_allocate(GtkWidget *widget, GtkAllocation *allocation);

static guint textbox_signals[LAST_SIGNAL] = { 0 };
static gint ui_skinned_textbox_textbox_press(GtkWidget *widget, GdkEventButton *event);
static gint ui_skinned_textbox_textbox_release(GtkWidget *widget, GdkEventButton *event);

static void ui_skinned_textbox_add(GtkContainer *container, GtkWidget *widget);
static void ui_skinned_textbox_toggle_doublesize(UiSkinnedTextbox *textbox);

static gint ui_skinned_textbox_motion_notify(GtkWidget *widget, GdkEventMotion *event);
static void ui_skinned_textbox_paint(UiSkinnedTextbox *textbox);
static void ui_skinned_textbox_redraw(UiSkinnedTextbox *textbox);
static gboolean ui_skinned_textbox_should_scroll(UiSkinnedTextbox *textbox);
static void textbox_generate_xfont_pixmap(UiSkinnedTextbox *textbox, const gchar *pixmaptext);
static void textbox_generate_pixmap(UiSkinnedTextbox *textbox);
static void textbox_handle_special_char(gchar c, gint * x, gint * y);

GType ui_skinned_textbox_get_type (void) {
    static GType textbox_type = 0;

    if (!textbox_type) {
        static const GTypeInfo textbox_info = {
            sizeof (UiSkinnedTextboxClass),
            NULL,                /* base_init */
            NULL,                /* base_finalize */
            (GClassInitFunc) ui_skinned_textbox_class_init,
            NULL,                /* class_finalize */
            NULL,                /* class_data */
            sizeof (UiSkinnedTextbox),
            16,                /* n_preallocs */
            (GInstanceInitFunc) ui_skinned_textbox_init,
        };

        textbox_type = g_type_register_static (GTK_TYPE_BIN, "UiSkinnedTextbox", &textbox_info, 0);
    }
    return textbox_type;
}

static void ui_skinned_textbox_class_init (UiSkinnedTextboxClass *klass) {
    GObjectClass *gobject_class;
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;

    gobject_class = G_OBJECT_CLASS(klass);
    object_class = (GtkObjectClass*)klass;
    widget_class = (GtkWidgetClass*)klass;
    container_class = (GtkContainerClass*)klass;

    parent_class = g_type_class_peek_parent(klass);
    gobject_class->constructor = ui_skinned_textbox_constructor;
    object_class->destroy = ui_skinned_textbox_destroy;

    widget_class->realize = ui_skinned_textbox_realize;
    widget_class->unrealize = ui_skinned_textbox_unrealize;
    widget_class->map = ui_skinned_textbox_map;
    widget_class->unmap = ui_skinned_textbox_unmap;
    widget_class->size_allocate = ui_skinned_textbox_size_allocate;
    widget_class->expose_event = ui_skinned_textbox_expose;
    widget_class->button_press_event = ui_skinned_textbox_textbox_press;
    widget_class->button_release_event = ui_skinned_textbox_textbox_release;
    widget_class->motion_notify_event = ui_skinned_textbox_motion_notify;

    container_class->add = ui_skinned_textbox_add;

    klass->clicked = NULL;
    klass->double_clicked = NULL;
    klass->right_clicked = NULL;
    klass->doubled = ui_skinned_textbox_toggle_doublesize;
    klass->redraw = ui_skinned_textbox_redraw;

    textbox_signals[CLICKED] = 
        g_signal_new ("clicked", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (UiSkinnedTextboxClass, clicked), NULL, NULL,
                      gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);

    textbox_signals[DOUBLE_CLICKED] = 
        g_signal_new ("double-clicked", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (UiSkinnedTextboxClass, double_clicked), NULL, NULL,
                      gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);

    textbox_signals[RIGHT_CLICKED] = 
        g_signal_new ("right-clicked", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (UiSkinnedTextboxClass, right_clicked), NULL, NULL,
                      gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);

    textbox_signals[DOUBLED] = 
        g_signal_new ("toggle-double-size", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (UiSkinnedTextboxClass, doubled), NULL, NULL,
                      gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);

    textbox_signals[REDRAW] = 
        g_signal_new ("redraw", G_OBJECT_CLASS_TYPE (object_class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (UiSkinnedTextboxClass, redraw), NULL, NULL,
                      gtk_marshal_VOID__VOID, G_TYPE_NONE, 0);

    g_type_class_add_private (gobject_class, sizeof (UiSkinnedTextboxPrivate));
}

static void ui_skinned_textbox_init (UiSkinnedTextbox *textbox) {
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    mutex = g_mutex_new();
    priv->image = gtk_image_new();
    priv->resize_width = 0;
    priv->resize_height = 0;
    priv->move_x = 0;
    priv->move_y = 0;
    textbox->redraw = TRUE;

    g_object_set (priv->image, "visible", TRUE, NULL);
    gtk_container_add(GTK_CONTAINER(GTK_WIDGET(textbox)), priv->image);

    GTK_WIDGET_SET_FLAGS (textbox, GTK_NO_WINDOW);
}

static void ui_skinned_textbox_destroy (GtkObject *object) {
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static GObject* ui_skinned_textbox_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_params) {
    GObject *object = (*G_OBJECT_CLASS (parent_class)->constructor)(type, n_construct_properties, construct_params);

    return object;
}

static void ui_skinned_textbox_realize (GtkWidget *widget) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    GdkWindowAttr attrib;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attrib.window_type = GDK_WINDOW_CHILD;
    attrib.x = widget->allocation.x;
    attrib.y = widget->allocation.y;
    attrib.width = widget->allocation.width;
    attrib.height = widget->allocation.height;
    attrib.wclass = GDK_INPUT_ONLY;
    attrib.event_mask = gtk_widget_get_events (widget);
    attrib.event_mask |= (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);

    widget->window = gtk_widget_get_parent_window(widget);
    g_object_ref(widget->window);

    textbox->event_window = gdk_window_new(gtk_widget_get_parent_window(widget), &attrib, GDK_WA_X | GDK_WA_Y);
    gdk_window_set_user_data (textbox->event_window, textbox);
}

static void ui_skinned_textbox_unrealize(GtkWidget *widget) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);

    if (textbox->event_window) {
        gdk_window_set_user_data (textbox->event_window, NULL);
        gdk_window_destroy (textbox->event_window);
        textbox->event_window = NULL;
    }

    GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void ui_skinned_textbox_map(GtkWidget *widget) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    g_return_if_fail (UI_IS_SKINNED_TEXTBOX (widget));
    GTK_WIDGET_CLASS (parent_class)->map(widget);
    gdk_window_show (textbox->event_window);
}

static void ui_skinned_textbox_unmap (GtkWidget *widget) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    g_return_if_fail (UI_IS_SKINNED_TEXTBOX(widget));

    if (textbox->event_window)
        gdk_window_hide (textbox->event_window);

    GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static gboolean ui_skinned_textbox_expose(GtkWidget *widget, GdkEventExpose *event) {
    if (GTK_WIDGET_DRAWABLE (widget)) {
        ui_skinned_textbox_paint(UI_SKINNED_TEXTBOX(widget));
        (*GTK_WIDGET_CLASS (parent_class)->expose_event) (widget, event);
    }

    return FALSE;
}

GtkWidget* ui_skinned_textbox_new () {
    return g_object_new (UI_TYPE_SKINNED_TEXTBOX, NULL);
}

void ui_skinned_textbox_setup(GtkWidget *widget, GtkWidget *fixed,GdkPixmap * parent, GdkGC * gc, gint x, gint y, gint w, gboolean allow_scroll, SkinPixmapId si) {

    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX(widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE(textbox);
    textbox->height = bmp_active_skin->properties.textbox_bitmap_font_height;
    textbox->x = x;
    textbox->y = y;
    textbox->text = g_strdup("");
    priv->gc = gc;
    textbox->width = w;
    priv->scroll_allowed = allow_scroll;
    priv->scroll_enabled = TRUE;
    priv->skin_index = si;
    priv->nominal_y = y;
    priv->nominal_height = textbox->height;
    priv->scroll_timeout = 0;
    priv->scroll_dummy = 0;

    priv->fixed = fixed;
    priv->double_size = FALSE;

    gtk_widget_set_size_request(widget, textbox->width, textbox->height);
    gtk_fixed_put(GTK_FIXED(priv->fixed), widget, textbox->x, textbox->y);
}

static void ui_skinned_textbox_size_allocate(GtkWidget *widget, GtkAllocation *allocation) {
    g_mutex_lock(mutex);
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    GtkAllocation child_alloc;

    widget->allocation = *allocation;
    if (GTK_BIN (textbox)->child) {                //image should fill whole widget
        child_alloc.x = widget->allocation.x;
        child_alloc.y = widget->allocation.y;
        child_alloc.width = MAX (1, widget->allocation.width);
        child_alloc.height = MAX (1, widget->allocation.height);

        gtk_widget_size_allocate (GTK_BIN (textbox)->child, &child_alloc);
    }

    if (GDK_IS_WINDOW(textbox->event_window))
        gdk_window_move_resize (textbox->event_window, widget->allocation.x, widget->allocation.y,
                                widget->allocation.width, widget->allocation.height);

    textbox->x = widget->allocation.x/(priv->double_size ? 2 : 1);
    textbox->y = widget->allocation.y/(priv->double_size ? 2 : 1);
    priv->move_x = 0;
    priv->move_y = 0;

    if (textbox->width != widget->allocation.width/(priv->double_size ? 2 : 1)) {
        textbox->width = widget->allocation.width/(priv->double_size ? 2 : 1);
        priv->resize_width = 0;
        priv->resize_height = 0;
        if (priv->pixmap_text) g_free(priv->pixmap_text);
        priv->pixmap_text = NULL;
        priv->offset = 0;
        textbox->redraw = TRUE;
        gtk_widget_queue_draw(GTK_WIDGET(textbox));
    }
    g_mutex_unlock(mutex);
}

static gboolean ui_skinned_textbox_textbox_press(GtkWidget *widget, GdkEventButton *event) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX(widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);

    if (event->type == GDK_BUTTON_PRESS) {
        textbox = UI_SKINNED_TEXTBOX(widget);

        if (event->button == 1) {
            if (priv->scroll_allowed) {
                if ((priv->pixmap_width > textbox->width) && priv->is_scrollable) {
                    priv->is_dragging = TRUE;
                    textbox->redraw = TRUE;
                    priv->drag_off = priv->offset;
                    priv->drag_x = event->x;
                }
            } else
                g_signal_emit(widget, textbox_signals[CLICKED], 0);

        } else if (event->button == 3) {
            g_signal_emit(widget, textbox_signals[RIGHT_CLICKED], 0);
        }
    } else if (event->type == GDK_2BUTTON_PRESS) {
        if (event->button == 1) {
            g_signal_emit(widget, textbox_signals[DOUBLE_CLICKED], 0);
        }
    }
    return TRUE;
}

static gboolean ui_skinned_textbox_textbox_release(GtkWidget *widget, GdkEventButton *event) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX(widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    if (event->button == 1) {
        priv->is_dragging = FALSE;
        textbox->redraw = TRUE;
    }

    return TRUE;
}

static gboolean ui_skinned_textbox_motion_notify(GtkWidget *widget, GdkEventMotion *event) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX(widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);

    if (priv->is_dragging) {
        if (priv->scroll_allowed &&
            priv->pixmap_width > textbox->width) {
            priv->offset = priv->drag_off - (event->x - priv->drag_x);

            while (priv->offset < 0)
                priv->offset = 0;

            while (priv->offset > (priv->pixmap_width - textbox->width))
                priv->offset = (priv->pixmap_width - textbox->width);

            textbox->redraw = TRUE;
            gtk_widget_queue_draw(widget);
        }
    }

    return FALSE;
}

static void ui_skinned_textbox_add(GtkContainer *container, GtkWidget *widget) {
    GTK_CONTAINER_CLASS (parent_class)->add(container, widget);
}

static void ui_skinned_textbox_toggle_doublesize(UiSkinnedTextbox *textbox) {
    GtkWidget *widget = GTK_WIDGET (textbox);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    priv->double_size = !priv->double_size;

    gtk_widget_set_size_request(widget, textbox->width*(1+priv->double_size), textbox->height*(1+priv->double_size));
    gtk_widget_set_uposition(widget, textbox->x*(1+priv->double_size), textbox->y*(1+priv->double_size));

    textbox->redraw = TRUE;
    gtk_widget_queue_draw(GTK_WIDGET(textbox));
}

static void ui_skinned_textbox_paint(UiSkinnedTextbox *textbox) {
    GtkWidget *widget = GTK_WIDGET (textbox);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);

    if (textbox->redraw == TRUE) {
        textbox->redraw = FALSE;
        gint cw;
        GdkPixmap *obj;

        if (textbox->text && (!priv->pixmap_text || strcmp(textbox->text, priv->pixmap_text)))
            textbox_generate_pixmap(textbox);

        if (priv->pixmap) {
            if (skin_get_id() != priv->skin_id) {
                priv->skin_id = skin_get_id();
                textbox_generate_pixmap(textbox);
            }
            obj = gdk_pixmap_new(NULL, textbox->width, textbox->height, gdk_rgb_get_visual()->depth);

            if(cfg.twoway_scroll) { // twoway scroll
                cw = priv->pixmap_width - priv->offset;
                if (cw > textbox->width)
                    cw = textbox->width;
                gdk_draw_drawable(obj, priv->gc, priv->pixmap, priv->offset, 0, 0, 0, cw, textbox->height);
                if (cw < textbox->width)
                    gdk_draw_drawable(obj, priv->gc, priv->pixmap, 0, 0,
                                      textbox->x + cw, textbox->y,
                                      textbox->width - cw, textbox->height);

                if (priv->double_size) {
                    GdkImage *img, *img2x;
                    img = gdk_drawable_get_image(obj, 0, 0, textbox->width, textbox->height);
                    img2x = create_dblsize_image(img);
                    gtk_image_set(GTK_IMAGE(priv->image), img2x, NULL);
                    g_object_unref(img2x);
                    g_object_unref(img);
                } else
                    gtk_image_set_from_pixmap(GTK_IMAGE(priv->image), obj, NULL);
            }
            else { // oneway scroll
                int cw1, cw2;

                if(priv->offset >= priv->pixmap_width)
                    priv->offset = 0;

                if(priv->pixmap_width - priv->offset > textbox->width){ // case1
                    cw1 = textbox->width;
                    gdk_draw_drawable(obj, priv->gc, priv->pixmap, priv->offset, 0,
                                      0, 0, cw1, textbox->height);
                }
                else { // case 2
                    cw1 = priv->pixmap_width - priv->offset;
                    gdk_draw_drawable(obj, priv->gc, priv->pixmap, priv->offset, 0,
                                      0, 0, cw1, textbox->height);
                    cw2 = textbox->width - cw1;
                    gdk_draw_drawable(obj, priv->gc, priv->pixmap, 0, 0, cw1, 0, cw2, textbox->height);
                }

                if (priv->double_size) {
                    GdkImage *img, *img2x;
                    img = gdk_drawable_get_image(obj, 0, 0, textbox->width, textbox->height);
                    img2x = create_dblsize_image(img);
                    gtk_image_set(GTK_IMAGE(priv->image), img2x, NULL);
                    g_object_unref(img2x);
                    g_object_unref(img);
                } else {
                    gtk_image_set_from_pixmap(GTK_IMAGE(priv->image), obj, NULL);
                }
            }

            g_object_unref(obj);
            gtk_widget_queue_resize(widget);
        }
    }
}

static void ui_skinned_textbox_redraw(UiSkinnedTextbox *textbox) {
    g_mutex_lock(mutex);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    if (priv->resize_width || priv->resize_height)
        gtk_widget_set_size_request(GTK_WIDGET(textbox),
                                   (textbox->width+priv->resize_width)*(1+priv->double_size),
                                   (textbox->height+priv->resize_height)*(1+priv->double_size));
    if (priv->move_x || priv->move_y)
        gtk_fixed_move(GTK_FIXED(priv->fixed), GTK_WIDGET(textbox), textbox->x+priv->move_x, textbox->y+priv->move_y);

    textbox->redraw = TRUE;
    gtk_widget_queue_draw(GTK_WIDGET(textbox));
    g_mutex_unlock(mutex);
}

static gboolean ui_skinned_textbox_should_scroll(UiSkinnedTextbox *textbox) {
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    if (!priv->scroll_allowed)
        return FALSE;

    if (priv->font) {
        gint width;
        text_get_extents(priv->fontname, textbox->text, &width, NULL, NULL, NULL);

        if (width <= textbox->width)
            return FALSE;
        else
            return TRUE;
    }

    if (g_utf8_strlen(textbox->text, -1) * bmp_active_skin->properties.textbox_bitmap_font_width > textbox->width)
        return TRUE;

    return FALSE;
}

void ui_skinned_textbox_set_xfont(GtkWidget *widget, gboolean use_xfont, const gchar * fontname) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    gint ascent, descent;

    g_return_if_fail(textbox != NULL);

    if (priv->font) {
        pango_font_description_free(priv->font);
        priv->font = NULL;
    }

    textbox->y = priv->nominal_y;
    textbox->height = priv->nominal_height;

    /* Make sure the pixmap is regenerated */
    if (priv->pixmap_text) {
        g_free(priv->pixmap_text);
        priv->pixmap_text = NULL;
    }

    if (!use_xfont || strlen(fontname) == 0)
        return;

    priv->font = pango_font_description_from_string(fontname);
    priv->fontname = g_strdup(fontname);

    text_get_extents(fontname,
                     "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz ",
                     NULL, NULL, &ascent, &descent);
    priv->font_ascent = ascent;
    priv->font_descent = descent;


    if (priv->font == NULL)
        return;

    textbox->height = priv->font_ascent;
    if (textbox->height > priv->nominal_height)
        textbox->y -= (textbox->height - priv->nominal_height) / 2;
    else
        textbox->height = priv->nominal_height;
}

void ui_skinned_textbox_set_text(GtkWidget *widget, const gchar *text) {
    g_return_if_fail(text != NULL);
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);

    if (!strcmp(textbox->text, text))
         return;
    if (textbox->text)
        g_free(textbox->text);

    textbox->text = str_to_utf8(text);
    priv->scroll_back = FALSE;
    textbox->redraw = TRUE;
    gtk_widget_queue_draw(GTK_WIDGET(textbox));
}

static void textbox_generate_xfont_pixmap(UiSkinnedTextbox *textbox, const gchar *pixmaptext) {
    gint length, i;
    GdkGC *gc, *maskgc;
    GdkColor *c, pattern;
    GdkBitmap *mask;
    PangoLayout *layout;
    gint width;
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);

    g_return_if_fail(textbox != NULL);
    g_return_if_fail(pixmaptext != NULL);

    length = g_utf8_strlen(pixmaptext, -1);

    text_get_extents(priv->fontname, pixmaptext, &width, NULL, NULL, NULL);

    priv->pixmap_width = MAX(width, textbox->width);
    priv->pixmap = gdk_pixmap_new(mainwin->window, priv->pixmap_width,
                                   textbox->height,
                                   gdk_rgb_get_visual()->depth);
    gc = priv->gc;
    c = skin_get_color(bmp_active_skin, SKIN_TEXTBG);
    for (i = 0; i < textbox->height; i++) {
        gdk_gc_set_foreground(gc, &c[6 * i / textbox->height]);
        gdk_draw_line(priv->pixmap, gc, 0, i, priv->pixmap_width, i);
    }

    mask = gdk_pixmap_new(mainwin->window, priv->pixmap_width, textbox->height, 1);
    maskgc = gdk_gc_new(mask);
    pattern.pixel = 0;
    gdk_gc_set_foreground(maskgc, &pattern);

    gdk_draw_rectangle(mask, maskgc, TRUE, 0, 0, priv->pixmap_width, textbox->height);
    pattern.pixel = 1;
    gdk_gc_set_foreground(maskgc, &pattern);

    gdk_gc_set_foreground(gc, skin_get_color(bmp_active_skin, SKIN_TEXTFG));

    layout = gtk_widget_create_pango_layout(mainwin, pixmaptext);
    pango_layout_set_font_description(layout, priv->font);

    gdk_draw_layout(priv->pixmap, gc, 0, (priv->font_descent / 2), layout);
    g_object_unref(layout);

    g_object_unref(maskgc);

    gdk_gc_set_clip_mask(gc, mask);
    c = skin_get_color(bmp_active_skin, SKIN_TEXTFG);
    for (i = 0; i < textbox->height; i++) {
        gdk_gc_set_foreground(gc, &c[6 * i / textbox->height]);
        gdk_draw_line(priv->pixmap, gc, 0, i, priv->pixmap_width, i);
    }
    g_object_unref(mask);
    gdk_gc_set_clip_mask(gc, NULL);
}

static gboolean textbox_scroll(gpointer data) {
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX(data);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (data);

    if (!priv->is_dragging) {
        if (priv->scroll_dummy < TEXTBOX_SCROLL_WAIT)
            priv->scroll_dummy++;
        else {
            if(cfg.twoway_scroll) {
                if (priv->scroll_back)
                    priv->offset -= 1;
                else
                    priv->offset += 1;

                if (priv->offset >= (priv->pixmap_width - textbox->width)) {
                    priv->scroll_back = TRUE;
                    priv->scroll_dummy = 0;
                }
                if (priv->offset <= 0) {
                    priv->scroll_back = FALSE;
                    priv->scroll_dummy = 0;
                }
            }
            else { // oneway scroll
                priv->scroll_back = FALSE;
                priv->offset += 1;
            }

            textbox->redraw = TRUE;
            gtk_widget_queue_draw(GTK_WIDGET(textbox));
        }
    }
    return TRUE;
}

static void textbox_generate_pixmap(UiSkinnedTextbox *textbox) {
    gint length, i, x, y, wl;
    gchar *pixmaptext;
    gchar *tmp, *stxt;
    GdkGC *gc;
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);
    g_return_if_fail(textbox != NULL);

    if (priv->pixmap) {
        g_object_unref(priv->pixmap);
        priv->pixmap = NULL;
    }

    /*
     * Don't reset the offset if only text after the last '(' has
     * changed.  This is a hack to avoid visual noice on vbr files
     * where we guess the length.
     */
    if (!(priv->pixmap_text && strrchr(textbox->text, '(') &&
          !strncmp(priv->pixmap_text, textbox->text,
                   strrchr(textbox->text, '(') - textbox->text)))
        priv->offset = 0;

    g_free(priv->pixmap_text);
    priv->pixmap_text = g_strdup(textbox->text);

    /*
     * wl is the number of (partial) letters visible. Only makes
     * sense when using skinned font.
     */
    wl = textbox->width / 5;
    if (wl * 5 != textbox->width)
        wl++;

    length = g_utf8_strlen(textbox->text, -1);

    priv->is_scrollable = FALSE;

    priv->is_scrollable = ui_skinned_textbox_should_scroll(textbox);

    if (!priv->is_scrollable && !priv->font && length <= wl) {
        gint pad = wl - length;
        gchar *padchars = g_strnfill(pad, ' ');

        pixmaptext = g_strconcat(priv->pixmap_text, padchars, NULL);
        g_free(padchars);
        length += pad;
    } else {
        if(cfg.twoway_scroll)
            pixmaptext = g_strdup(priv->pixmap_text);
        else
            pixmaptext = g_strdup_printf("%s *** ", priv->pixmap_text);
    }

    if (priv->is_scrollable) {
        if (priv->scroll_enabled && !priv->scroll_timeout) {
            gint tag;
            tag = TEXTBOX_SCROLL_SMOOTH_TIMEOUT;
            priv->scroll_timeout = g_timeout_add(tag, textbox_scroll, textbox);
        }
    } else {
        if (priv->scroll_timeout) {
            g_source_remove(priv->scroll_timeout);
            priv->scroll_timeout = 0;
        }
        priv->offset = 0;
    }

    if (priv->font) {
        textbox_generate_xfont_pixmap(textbox, pixmaptext);
        g_free(pixmaptext);
        return;
    }

    priv->pixmap_width = length * bmp_active_skin->properties.textbox_bitmap_font_width;
    priv->pixmap = gdk_pixmap_new(NULL,
                                     priv->pixmap_width, bmp_active_skin->properties.textbox_bitmap_font_height,
                                     gdk_rgb_get_visual()->depth);
    gc = priv->gc;

    for (tmp = stxt = g_utf8_strup(pixmaptext, -1), i = 0;
         tmp != NULL && i < length; i++, tmp = g_utf8_next_char(tmp)) {
        gchar c = *tmp;
        x = y = -1;
        if (c >= 'A' && c <= 'Z') {
            x = bmp_active_skin->properties.textbox_bitmap_font_width * (c - 'A');
            y = 0;
        }
        else if (c >= '0' && c <= '9') {
            x = bmp_active_skin->properties.textbox_bitmap_font_width * (c - '0');
            y = bmp_active_skin->properties.textbox_bitmap_font_height;
        }
        else
            textbox_handle_special_char(c, &x, &y);

        skin_draw_pixmap(bmp_active_skin,
                         priv->pixmap, gc, priv->skin_index,
                         x, y, i * bmp_active_skin->properties.textbox_bitmap_font_width, 0,
                         bmp_active_skin->properties.textbox_bitmap_font_width, 
                         bmp_active_skin->properties.textbox_bitmap_font_height);
    }
    g_free(stxt);
    g_free(pixmaptext);
}

void ui_skinned_textbox_set_scroll(GtkWidget *widget, gboolean scroll) {
    g_return_if_fail(widget != NULL);
    UiSkinnedTextbox *textbox = UI_SKINNED_TEXTBOX (widget);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (textbox);

    priv->scroll_enabled = scroll;
    if (priv->scroll_enabled && priv->is_scrollable && priv->scroll_allowed) {
        gint tag;
        tag = TEXTBOX_SCROLL_SMOOTH_TIMEOUT;
        if (priv->scroll_timeout) {
            g_source_remove(priv->scroll_timeout);
            priv->scroll_timeout = 0;
        }
        priv->scroll_timeout = g_timeout_add(tag, textbox_scroll, textbox);

    } else {

        if (priv->scroll_timeout) {
            g_source_remove(priv->scroll_timeout);
            priv->scroll_timeout = 0;
        }

        priv->offset = 0;
        textbox->redraw = TRUE;
        gtk_widget_queue_draw(GTK_WIDGET(textbox));
    }
}

static void
textbox_handle_special_char(gchar c, gint * x, gint * y)
{
    gint tx, ty;

    switch (c) {
    case '"':
        tx = 26;
        ty = 0;
        break;
    case '\r':
        tx = 10;
        ty = 1;
        break;
    case ':':
    case ';':
        tx = 12;
        ty = 1;
        break;
    case '(':
        tx = 13;
        ty = 1;
        break;
    case ')':
        tx = 14;
        ty = 1;
        break;
    case '-':
        tx = 15;
        ty = 1;
        break;
    case '`':
    case '\'':
        tx = 16;
        ty = 1;
        break;
    case '!':
        tx = 17;
        ty = 1;
        break;
    case '_':
        tx = 18;
        ty = 1;
        break;
    case '+':
        tx = 19;
        ty = 1;
        break;
    case '\\':
        tx = 20;
        ty = 1;
        break;
    case '/':
        tx = 21;
        ty = 1;
        break;
    case '[':
        tx = 22;
        ty = 1;
        break;
    case ']':
        tx = 23;
        ty = 1;
        break;
    case '^':
        tx = 24;
        ty = 1;
        break;
    case '&':
        tx = 25;
        ty = 1;
        break;
    case '%':
        tx = 26;
        ty = 1;
        break;
    case '.':
    case ',':
        tx = 27;
        ty = 1;
        break;
    case '=':
        tx = 28;
        ty = 1;
        break;
    case '$':
        tx = 29;
        ty = 1;
        break;
    case '#':
        tx = 30;
        ty = 1;
        break;
    case '?':
        tx = 3;
        ty = 2;
        break;
    case '*':
        tx = 4;
        ty = 2;
        break;
    default:
        tx = 29;
        ty = 0;
        break;
    }

    *x = tx * bmp_active_skin->properties.textbox_bitmap_font_width;
    *y = ty * bmp_active_skin->properties.textbox_bitmap_font_height;
}

void ui_skinned_textbox_move_relative(GtkWidget *widget, gint x, gint y) {
    g_mutex_lock(mutex);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (widget);
    priv->move_x += x;
    priv->move_y += y;
    g_mutex_unlock(mutex);
}

void ui_skinned_textbox_resize_relative(GtkWidget *widget, gint w, gint h) {
    g_mutex_lock(mutex);
    UiSkinnedTextboxPrivate *priv = UI_SKINNED_TEXTBOX_GET_PRIVATE (widget);
    priv->resize_width += w;
    priv->resize_height += h;
    g_mutex_unlock(mutex);
}
