/* monkey-canvas.h
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

#ifndef MONKEY_CANVAS_H
#define MONKEY_CANVAS_H

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_MONKEY_CANVAS      (monkey_canvas_get_type())

#define MONKEY_CANVAS(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_MONKEY_CANVAS,MonkeyCanvas))
#define MONKEY_CANVAS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MONKEY_CANVAS,MonkeyCanvasClass))
#define IS_MONKEY_CANVAS(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_MONKEY_CANVAS))
#define IS_MONKEY_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MONKEY_CANVAS))
#define MONKEY_CANVAS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MONKEY_CANVAS, MonkeyCanvasClass))



typedef struct MonkeyCanvasPrivate MonkeyCanvasPrivate;



typedef struct {
  GtkDrawingArea parent_instance;
  MonkeyCanvasPrivate * private;
} MonkeyCanvas ;

typedef struct {
  GtkDrawingAreaClass parent_class;
} MonkeyCanvasClass;

typedef struct Block Block;
typedef struct Layer Layer;

GType monkey_canvas_get_type(void);

MonkeyCanvas * monkey_canvas_new(void);


Block * monkey_canvas_create_block_from_image(MonkeyCanvas * canvas,
					    const char * path,
					    gint x_size,
					    gint y_size,
					    gint x_center,
					    gint y_center);

Layer * monkey_canvas_get_root_layer(MonkeyCanvas * canvas);

Layer * monkey_canvas_append_layer(MonkeyCanvas * canvas,
				 gdouble x,gdouble y);

void monkey_canvas_add_block(MonkeyCanvas * canvas,
			  Layer * layer,
			  Block * block,
			  gdouble x,
			  gdouble y);

void monkey_canvas_move_block(MonkeyCanvas * canvas,
			   Block * block,
			   gdouble x,
			   gdouble y);

void monkey_canvas_remove_block(MonkeyCanvas * canvas,
			     Block * block);


void monkey_canvas_unref_block(MonkeyCanvas * canvas,
			    Block * b);


void monkey_canvas_clear(MonkeyCanvas * monkey_canvas);
void monkey_canvas_paint(MonkeyCanvas * monkey_canvas);

void block_get_position(Block * block,
			gdouble *x,
			gdouble *y);

G_END_DECLS





#endif
