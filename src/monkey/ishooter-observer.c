/* ishooter-observer.c - 
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

#include "shooter.h"

static guint ishooter_observer_base_init_count = 0;

struct IShooterObserverClassPrivate {

  void (* rotated) (IShooterObserver * so,
		    Shooter * shooter);

  void (* shoot) (IShooterObserver * so,
		  Shooter * shooter,
		  Bubble * b);

  void (* bubble_added) (IShooterObserver * so,
			 Shooter * s,
			 Bubble * b);
};

void ishooter_observer_class_virtual_init(IShooterObserverClass * class,
					  void (* rotated) (IShooterObserver * so,
							    Shooter * shooter),

					  void (* shoot) (IShooterObserver * so,
							  Shooter * shooter,
							  Bubble * b),
					  void (* bubble_added) (IShooterObserver * so,
								 Shooter * s,
								 Bubble * b)) {

  class->private->rotated = rotated;
  class->private->shoot = shoot;
  class->private->bubble_added = bubble_added;
}

static void ishooter_observer_base_init(IShooterObserverClass* file) {
    ishooter_observer_base_init_count++;
    file->private = g_new0(IShooterObserverClassPrivate,1);
    if (ishooter_observer_base_init_count == 1) {
	/* add signals here */
    }
}

static void ishooter_observer_base_finalize(IShooterObserverClass* file) {
    ishooter_observer_base_init_count--;
    if (ishooter_observer_base_init_count == 0) {
	/* destroy signals here */
    }
}

GType ishooter_observer_get_type(void) {

  static GType ishooter_observer_type = 0;
  if (!ishooter_observer_type) {
    static const GTypeInfo ishooter_observer_info = {
      sizeof(IShooterObserverClass),
      (GBaseInitFunc) ishooter_observer_base_init,
      (GBaseFinalizeFunc) ishooter_observer_base_finalize
    };
    
    ishooter_observer_type = g_type_register_static(G_TYPE_INTERFACE, 
						   "IShooterObserver", 
						   &ishooter_observer_info, 
						   0);

    g_type_interface_add_prerequisite(ishooter_observer_type, G_TYPE_OBJECT);
  }
  return ishooter_observer_type;
}


void ishooter_observer_rotated(IShooterObserver * so,
			      Shooter * shooter) {
  IShooterObserverClass * iface;
  g_assert(IS_ISHOOTER_OBSERVER(so));
  g_assert(G_IS_OBJECT(so));

  iface = ISHOOTER_OBSERVER_GET_IFACE(so);
  iface->private->rotated(so,shooter);
}

void ishooter_observer_shoot (IShooterObserver * so,
			      Shooter * shooter,
			      Bubble * b) {
  IShooterObserverClass * iface;
  g_assert(IS_ISHOOTER_OBSERVER(so));
  g_assert(G_IS_OBJECT(so));

  iface = ISHOOTER_OBSERVER_GET_IFACE(so);
  iface->private->shoot(so,shooter,b);
}

void ishooter_observer_bubble_added(IShooterObserver *so,
				    Shooter * shooter,
				    Bubble * b) {

  IShooterObserverClass * iface;
  g_assert(IS_ISHOOTER_OBSERVER(so));
  g_assert(G_IS_OBJECT(so));

  iface = ISHOOTER_OBSERVER_GET_IFACE(so);
  iface->private->bubble_added(so,shooter,b);
}

