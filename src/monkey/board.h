/* board.h
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

#ifndef _BOARD_H
#define _BOARD_H

#include <gtk/gtk.h>
#include "bubble.h"

#define TYPE_BOARD            (board_get_type())

#define BOARD(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_BOARD,Board))
#define BOARD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_BOARD,BoardClass))
#define IS_BOARD(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_BOARD))
#define IS_BOARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_BOARD))
#define BOARD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_BOARD, BoardClass))

typedef struct BoardPrivate BoardPrivate;

#define TYPE_IBOARD_OBSERVER        (iboard_observer_get_type())

#define IBOARD_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_IBOARD_OBSERVER,IBoardObserver))
#define IS_IBOARD_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_IBOARD_OBSERVER))

#define IBOARD_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IBOARD_OBSERVER, IBoardObserverClass))
#define IBOARD_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TYPE_IBOARD_OBSERVER, IBoardObserverClass))


typedef struct IBoardObserver IBoardObserver;

typedef struct {
  GObject parent_instance;
  BoardPrivate * private;
} Board;

typedef struct {
  GObjectClass parent_class;
} BoardClass;

GType board_get_type(void);

typedef struct {
  gint x;
  gint y;
} Cell_xy;

Board * board_new(gdouble y_min,const gchar * filename,gint level);

void board_init(Board * board,Bubble ** bubbles,gint count);

void board_add_bubbles(Board *board,
		       Bubble ** bubbles);

void board_insert_bubbles(Board *board,
			  Bubble ** bubbles);

Bubble * board_get_bubble_at(Board * board,
			     gint x,gint y);

gint * board_get_colors_count(Board * b);
gint board_get_row_count(Board * b);
gint board_get_column_count(Board * b);

void board_stick_bubble(Board *board,Bubble *bubble,gint time);
gboolean board_collide_bubble(Board * board,Bubble *b);
gboolean board_is_lost(Board * board);

gdouble board_get_y_min(Board * board);

void board_down( Board * board);

int board_bubbles_count(Board * board);

void board_attach_observer(Board * board,IBoardObserver * bo);
void board_detach_observer(Board * board,IBoardObserver * bo);

typedef struct IBoardObserverClassPrivate IBoardObserverClassPrivate;

typedef struct {
  GTypeInterface root_interface;
  IBoardObserverClassPrivate * private;
} IBoardObserverClass;


GType iboard_observer_get_type(void);

void iboard_observer_class_virtual_init(IBoardObserverClass * class,
					void (* bubbles_exploded )(IBoardObserver * bo,
								   Board *board,
								   GList * exploded,
								   GList * fallen),
					void (* bubbles_added) (IBoardObserver * po,
								Board * board,
								GList * bubbles),
					void (* bubble_sticked)(IBoardObserver * po,
								Board * board,
								Bubble * bubble,
								gint stiked),
					void (* bubbles_inserted) (IBoardObserver * po,
								   Board * board,
								   Bubble ** bubbles,
								   int count),
					void (* down ) (IBoardObserver * po,
							Board * board));

void iboard_observer_down(IBoardObserver * bo,
			  Board * board);

void iboard_observer_bubbles_exploded(IBoardObserver * bo,
				     Board *board,
				     GList * exploded,
				     GList * fallen);

void iboard_observer_bubbles_added(IBoardObserver * bo,
				   Board * board,
				   GList * bubbles);

void iboard_observer_bubbles_inserted(IBoardObserver * bo,
				      Board * board,
				      Bubble ** bubbles,
				      int count);

void iboard_observer_bubble_sticked(IBoardObserver * bo,
				    Board * board,
				    Bubble * sticked_bubble,
				    gint time);
#endif
