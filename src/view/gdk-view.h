/* gdk-view.h
 * Copyright (C) 2002 Laurent Belmonte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GDK_VIEW_H
#define GDK_VIEW_H

#include <gtk/gtk.h>
#include "color.h"
#include "gdk-canvas.h"
#include "monkey.h"

G_BEGIN_DECLS

#define TYPE_GDK_VIEW            (gdk_view_get_type())

#define GDK_VIEW(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GDK_VIEW,GdkView))
#define GDK_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GDK_VIEW,GdkViewClass))
#define IS_GDK_VIEW(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GDK_VIEW))
#define IS_GDK_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GDK_VIEW))
#define GDK_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GDK_VIEW, GdkViewClass))

typedef struct GdkViewPrivate GdkViewPrivate;

typedef struct {
  GObject parent_instance;
  GdkViewPrivate * private;
} GdkView;

typedef struct {
  GObjectClass parent_class;
} GdkViewClass;


GType gdk_view_get_type(void);

GdkView * gdk_view_new(GdkCanvas * canvas,Monkey * monkey,
		       gint x,gint y,gboolean back_needed);

void gdk_view_update(GdkView * gdk_view,
		     gint time );

void gdk_view_set_gems_count(GdkView * d,int gems);
void gdk_view_set_score(GdkView * d,int score);

void gdk_view_set_waiting_bubbles(GdkView * d,int bubbles);


void HACK_load_contents();
void gdk_view_draw_lost(GdkView *d);
void gdk_view_draw_win(GdkView *d);

G_END_DECLS
#endif
