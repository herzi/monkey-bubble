/* bubble.h
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

#ifndef BUBBLE_H
#define BUBBLE_H

#include <gtk/gtk.h>
#include "color.h"
G_BEGIN_DECLS


#define BUBBLE_RADIUS 16
#define TYPE_BUBBLE            (bubble_get_type())

#define BUBBLE(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_BUBBLE,Bubble))
#define BUBBLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_BUBBLE,BubbleClass))
#define IS_BUBBLE(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_BUBBLE))
#define IS_BUBBLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_BUBBLE))
#define BUBBLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_BUBBLE, BubbleClass))

typedef struct BubblePrivate BubblePrivate;

#define TYPE_IBUBBLE_OBSERVER        (ibubble_observer_get_type())

#define IBUBBLE_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_IBUBBLE_OBSERVER,IBubbleObserver))
#define IS_IBUBBLE_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_IBUBBLE_OBSERVER))

#define IBUBBLE_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IBUBBLE_OBSERVER, IBubbleObserverClass))
#define IBUBBLE_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TYPE_IBUBBLE_OBSERVER, IBubbleObserverClass))

typedef struct IBubbleObserver IBubbleObserver;


typedef struct {
  GObject parent_instance;
  BubblePrivate * private;
} Bubble;

typedef struct {
  GObjectClass parent_class;
} BubbleClass;


GType bubble_get_type(void);

Bubble * bubble_new( Color color,gdouble x,gdouble y);

void bubble_set_position(Bubble * bubble,
			    const gdouble x,
			    const gdouble y);

void bubble_get_position(const Bubble * bubble,
			    gdouble * x,
			    gdouble *y);

void bubble_set_velocity(Bubble * bubble,
			 gdouble vx,
			 gdouble vy);

void bubble_get_velocity(const Bubble * bubble,
			    gdouble * vx,
			    gdouble * vy);

Color bubble_get_color(const Bubble * bubble);
void bubble_attach_observer(Bubble * bubble,IBubbleObserver * bo);
void bubble_detach_observer(Bubble * bubble,IBubbleObserver * bo);

gboolean bubble_collide_bubble(const Bubble * bubble ,
			       const Bubble * bubble_to_colliside );



typedef struct IBubbleObserverPrivate IBubbleObserverPrivate;
typedef struct {
  GTypeInterface root_interface;
  IBubbleObserverPrivate * private;

} IBubbleObserverClass;


GType ibubble_observer_get_type(void);

void ibubble_observer_changed(IBubbleObserver * bo,
				 Bubble * bubble);
void ibubble_observer_class_virtual_init(IBubbleObserverClass * class,
				   void (*  changed)(IBubbleObserver *bo,
						     Bubble * bubble));

G_END_DECLS





#endif
