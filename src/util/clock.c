/* clock.c
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
#include <gtk/gtk.h>
#include "clock.h"

#define PRIVATE(clock) (clock->private)

typedef enum
{
  CLOCK_STOPPED =1,
  CLOCK_RUNNING = 2,
  CLOCK_PAUSED = 3
} ClockState;

static GObjectClass* parent_class = NULL;

struct ClockPrivate {
  GTimeVal reference_time;
  GTimeVal pause_time;
  ClockState state;
};



static void clock_instance_init(Clock * clock) {
  clock->private =g_new0 (ClockPrivate, 1);			
}

static void clock_finalize(GObject* object) {
  Clock * clock = CLOCK(object);


  g_free(PRIVATE(clock));

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void clock_class_init (ClockClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = clock_finalize;
}


GType clock_get_type(void) {
    static GType clock_type = 0;
    
    if (!clock_type) {
      static const GTypeInfo clock_info = {
	sizeof(ClockClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) clock_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(Clock),
	1,              /* n_preallocs */
	(GInstanceInitFunc) clock_instance_init,
      };


      
      clock_type = g_type_register_static(G_TYPE_OBJECT,
						"Clock",
						&clock_info, 0);
    }
    
    return clock_type;
}



Clock * clock_new(void) {
  Clock * clock;

  clock = CLOCK (g_object_new (TYPE_CLOCK, NULL));

  PRIVATE(clock)->state = CLOCK_STOPPED;
  return clock;
}

void clock_start(Clock * clock) {  

  g_assert( IS_CLOCK(clock));
  g_get_current_time( &(PRIVATE(clock)->reference_time) );
  PRIVATE(clock)->state = CLOCK_RUNNING;
}

void clock_pause(Clock * clock,gboolean paused) {
  GTimeVal c;
  GTimeVal p;
  GTimeVal r;

  g_assert( IS_CLOCK(clock));

  if( paused ) {

		//   g_assert( ( PRIVATE(clock)->state == CLOCK_RUNNING));
    PRIVATE(clock)->state = CLOCK_PAUSED;
    g_get_current_time( &(PRIVATE(clock)->pause_time ));
  } else {
    g_assert( PRIVATE(clock)->state == CLOCK_PAUSED);
    g_get_current_time(&c);
    
    p.tv_sec =PRIVATE(clock)->pause_time.tv_sec;
    p.tv_usec =PRIVATE(clock)->pause_time.tv_usec;
    
    r.tv_sec = PRIVATE(clock)->reference_time.tv_sec;
    r.tv_usec = PRIVATE(clock)->reference_time.tv_usec;
    
    
    r.tv_sec = r.tv_sec + ( c.tv_sec - p.tv_sec);
    r.tv_usec = r.tv_usec + ( c.tv_usec - p.tv_usec );
    
    r.tv_sec = r.tv_sec + r.tv_usec / 1000000;
    r.tv_usec %= 1000000;
    
    PRIVATE(clock)->reference_time.tv_sec = r.tv_sec;
    PRIVATE(clock)->reference_time.tv_usec = r.tv_usec;
    PRIVATE(clock)->state = CLOCK_RUNNING;
  }
}

gint clock_get_time(Clock * clock) {
  GTimeVal current_time;

  g_assert( IS_CLOCK(clock));
  
  switch( PRIVATE(clock)->state ) {
  case CLOCK_RUNNING : 
    g_get_current_time(&current_time);    
    return 
      ( current_time.tv_sec  - PRIVATE(clock)->reference_time.tv_sec)*1000
      + ( current_time.tv_usec - PRIVATE(clock)->reference_time.tv_usec) / 1000;
    break;
  case CLOCK_PAUSED :
    g_get_current_time(&current_time);    
    return 
      ( current_time.tv_sec  - PRIVATE(clock)->pause_time.tv_sec)*1000
      + ( current_time.tv_usec - PRIVATE(clock)->pause_time.tv_usec) / 1000;
    break;
  default :
    return 0;
        
  }
}


