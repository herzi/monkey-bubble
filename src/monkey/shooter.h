/* shoooter.h
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

#ifndef _SHOOTER_H
#define _SHOOTER_H

#include <gtk/gtk.h>
#include "bubble.h"

G_BEGIN_DECLS

#define TYPE_SHOOTER            (shooter_get_type())

#define SHOOTER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_SHOOTER,Shooter))
#define SHOOTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_SHOOTER,ShooterClass))
#define IS_SHOOTER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_SHOOTER))
#define IS_SHOOTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_SHOOTER))
#define SHOOTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_SHOOTER, ShooterClass))

typedef struct ShooterPrivate ShooterPrivate;

#define TYPE_ISHOOTER_OBSERVER        (ishooter_observer_get_type())

#define ISHOOTER_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_ISHOOTER_OBSERVER,IShooterObserver))
#define IS_ISHOOTER_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_ISHOOTER_OBSERVER))

#define ISHOOTER_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ISHOOTER_OBSERVER, IShooterObserverClass))

#define ISHOOTER_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TYPE_ISHOOTER_OBSERVER, IShooterObserverClass))

typedef struct IShooterObserver IShooterObserver;

typedef struct {
  GObject parent_instance;
  ShooterPrivate * private;
} Shooter;

typedef struct {
  GObjectClass parent_class;
} ShooterClass;

GType shooter_get_type(void);


Shooter * shooter_new(gdouble x_pos,gdouble y_pos,
		      gdouble min_angle,gdouble max_angle,
		      gdouble shoot_speed);

void shooter_add_bubble(Shooter * shooter,Bubble *b);

void shooter_get_position(const Shooter * shooter,gdouble * x,gdouble * y);

Bubble * shooter_shoot(Shooter * s);

Bubble * shooter_get_current_bubble(Shooter *s);
Bubble * shooter_get_waiting_bubble(Shooter *s);

gdouble shooter_get_angle(Shooter * s);

void shooter_set_angle(Shooter *s,gdouble angle);
void shooter_attach_observer(Shooter * s,IShooterObserver *so);
void shooter_detach_observer(Shooter * s,IShooterObserver *so);

typedef struct IShooterObserverClassPrivate IShooterObserverClassPrivate;

typedef struct {
  GTypeInterface root_interface;
  IShooterObserverClassPrivate * private;

} IShooterObserverClass;


GType ishooter_observer_get_type(void);

void ishooter_observer_class_virtual_init(IShooterObserverClass * class,
					  void (* rotated) (IShooterObserver * so,
							    Shooter * shooter),

					  void (* shoot) (IShooterObserver * so,
							  Shooter * shooter,
							  Bubble * b),
					  void (* bubble_added) (IShooterObserver * so,
								 Shooter * s,
								 Bubble * b));
void ishooter_observer_rotated(IShooterObserver * so,
			      Shooter * shooter);

void ishooter_observer_shoot (IShooterObserver * so,
			      Shooter * shooter,
			      Bubble * b);

void ishooter_observer_bubble_added(IShooterObserver *so,
				    Shooter * shooter,
				    Bubble * b);
#endif
