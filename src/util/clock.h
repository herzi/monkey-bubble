/* clock.h
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

#ifndef CLOCK_H
#define CLOCK_H

#include <gtk/gtk.h>
G_BEGIN_DECLS


#define TYPE_CLOCK            (clock_get_type())

#define CLOCK(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_CLOCK,Clock))
#define CLOCK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CLOCK,ClockClass))
#define IS_CLOCK(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_CLOCK))
#define IS_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CLOCK))
#define CLOCK_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CLOCK, ClockClass))

typedef struct ClockPrivate ClockPrivate;



typedef struct {
  GObject parent_instance;
  ClockPrivate * private;
} Clock;

typedef struct {
  GObjectClass parent_class;
} ClockClass;


GType clock_get_type(void);

Clock * clock_new(void);

void clock_start(Clock * clock);
void clock_pause(Clock * clock,gboolean paused);
gint clock_get_time(Clock * clock);

G_END_DECLS





#endif
