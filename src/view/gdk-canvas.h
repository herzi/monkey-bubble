/* gdk-canvas.h
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

#ifndef GDK_CANVAS_H
#define GDK_CANVAS_H

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_GDK_CANVAS      (gdk_canvas_get_type())

#define GDK_CANVAS(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GDK_CANVAS,GdkCanvas))
#define GDK_CANVAS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GDK_CANVAS,GdkCanvasClass))
#define IS_GDK_CANVAS(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GDK_CANVAS))
#define IS_GDK_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GDK_CANVAS))
#define GDK_CANVAS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GDK_CANVAS, GdkCanvasClass))



typedef struct GdkCanvasPrivate GdkCanvasPrivate;



typedef struct {
  GtkDrawingArea parent_instance;
  GdkCanvasPrivate * private;
} GdkCanvas ;

typedef struct {
  GtkDrawingAreaClass parent_class;
} GdkCanvasClass;

typedef struct Block Block;
typedef struct Layer Layer;

GType gdk_canvas_get_type(void);

GdkCanvas * gdk_canvas_new(void);


Block * gdk_canvas_create_block_from_image(GdkCanvas * canvas,
					    const char * path,
					    gint x_size,
					    gint y_size,
					    gint x_center,
					    gint y_center);

Layer * gdk_canvas_get_root_layer(GdkCanvas * canvas);

Layer * gdk_canvas_append_layer(GdkCanvas * canvas,
				 gdouble x,gdouble y);

void gdk_canvas_add_block(GdkCanvas * canvas,
			  Layer * layer,
			  Block * block,
			  gdouble x,
			  gdouble y);

void gdk_canvas_move_block(GdkCanvas * canvas,
			   Block * block,
			   gdouble x,
			   gdouble y);

void gdk_canvas_remove_block(GdkCanvas * canvas,
			     Block * block);


void gdk_canvas_unref_block(GdkCanvas * canvas,
			    Block * b);


void gdk_canvas_clear(GdkCanvas * gdk_canvas);
void gdk_canvas_paint(GdkCanvas * gdk_canvas);

void block_get_position(Block * block,
			gdouble *x,
			gdouble *y);
//GtkWidget * gdk_canvas_get_widget( GdkCanvas * gdk_canvas );

G_END_DECLS





#endif
