/* playground.h
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
#ifndef _PLAYGROUND_H
#define _PLAYGROUND_H

#include <gtk/gtk.h>

#include "bubble.h"
#include "board.h"


G_BEGIN_DECLS

#define TYPE_PLAYGROUND            (playground_get_type())

#define PLAYGROUND(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_PLAYGROUND,Playground))
#define PLAYGROUND_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PLAYGROUND,PlaygroundClass))
#define IS_PLAYGROUND(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_PLAYGROUND))
#define IS_PLAYGROUND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PLAYGROUND))
#define PLAYGROUND_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_PLAYGROUND, PlaygroundClass))

typedef struct PlaygroundPrivate PlaygroundPrivate;

#define TYPE_IPLAYGROUND_OBSERVER        (iplayground_observer_get_type())

#define IPLAYGROUND_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_IPLAYGROUND_OBSERVER,IPlaygroundObserver))
#define IS_IPLAYGROUND_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_IPLAYGROUND_OBSERVER))

#define IPLAYGROUND_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IPLAYGROUND_OBSERVER, IPlaygroundObserverClass))
#define IPLAYGROUND_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE  ((obj), TYPE_IPLAYGROUND_OBSERVER, IPlaygroundObserverClass))

typedef struct IPlaygroundObserver IPlaygroundObserver;

typedef struct {
  GObject parent_instance;
  PlaygroundPrivate * private;
} Playground;

typedef struct {
  GObjectClass parent_class;
} PlaygroundClass;

GType playground_get_type(void);

Playground * playground_new(gdouble max_x,gdouble min_x,
			    const gchar * filename,gint level);
Bubble * playground_get_active_bubble(Playground * playground);
gboolean playground_is_ready_for_shoot(Playground * playground);
Board * playground_get_board(Playground * playground );
void playground_update(Playground * playground,gint time);
void playground_shoot_bubble(Playground *pl,Bubble * b);
void playground_attach_observer(Playground *pl,IPlaygroundObserver * po);
void playground_detach_observer(Playground *pl,IPlaygroundObserver * po);


typedef struct IPlaygroundObserverClassPrivate IPlaygroundObserverClassPrivate;
typedef struct {
  GTypeInterface root_interface;
  IPlaygroundObserverClassPrivate * private;
} IPlaygroundObserverClass;


GType iplayground_observer_get_type(void);

void iplayground_observer_class_virtual_init(IPlaygroundObserverClass * i,
					     void (* bubble_shot )(IPlaygroundObserver *po,
								   Playground *pg,
								   Bubble * b),					     
					     void (* bubble_wall_collision )(IPlaygroundObserver * po,
									     Playground *pg),
					     void (* bubble_board_collision )(IPlaygroundObserver * po,
									      Playground *pg),
					     void (* game_lost)(IPlaygroundObserver * po,
								Playground * pg));
void iplayground_observer_bubble_shot(IPlaygroundObserver *po,
				      Playground *pg,
				      Bubble * b);

void iplayground_observer_bubble_wall_collision(IPlaygroundObserver * po,
						Playground *pg);

void iplayground_observer_bubble_board_collision(IPlaygroundObserver *po,
						 Playground * pg);

void iplayground_observer_game_lost(IPlaygroundObserver * po,	
				    Playground * pg);
#endif
