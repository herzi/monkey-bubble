/* monkey.h
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

#ifndef MONKEY_H
#define MONKEY_H

#include <gtk/gtk.h>
#include "playground.h"
#include "shooter.h"

G_BEGIN_DECLS

#define TYPE_MONKEY            (monkey_get_type())

#define MONKEY(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_MONKEY,Monkey))
#define MONKEY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MONKEY,MonkeyClass))
#define IS_MONKEY(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_MONKEY))
#define IS_MONKEY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MONKEY))
#define MONKEY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MONKEY, MonkeyClass))

typedef struct MonkeyPrivate MonkeyPrivate;
typedef struct IMonkeyObserver IMonkeyObserver;



typedef struct {
  GObject parent_instance;
  MonkeyPrivate * private;
} Monkey;

typedef struct {
  GObjectClass parent_class;
} MonkeyClass;


GType monkey_get_type(void);

/* constructor */
Monkey * monkey_new(void);
Monkey * monkey_new_level_from_file(const gchar * filename,gint level);

void monkey_left_changed( Monkey * monkey,gboolean pressed,
			  gint time);

void monkey_right_changed( Monkey * monkey,gboolean pressed,
			   gint time);

void monkey_update( Monkey * monkey,gint time );

Shooter * monkey_get_shooter(Monkey * monkey);
Playground * monkey_get_playground(Monkey * monkey);

void monkey_shoot(Monkey * monkey,gint time);

void monkey_set_board_down(Monkey * monkey);

/* bubbles added by other player */

//Bubble ** monkey_get_current_free_columns(Monkey * monkey);

/* 
 * @return array with column coordonate of the bubbles
 */
int * monkey_add_bubbles (
			 Monkey * monkey,
			 int  bubble_count,
			 Color * bubbles_colors);


void monkey_add_bubbles_at (
			   Monkey * monkey,
			   int bubble_count,
			   Color * bubbles_colors,
			   int * bubbles_column /* waiting column for bubbles */
			   );

gint monkey_get_shot_count(Monkey * monkey);

void monkey_insert_bubbles(Monkey * monnkey,Bubble ** bubbles_8);


gboolean monkey_is_empty(Monkey * monkey);

void monkey_attach_observer(Monkey * monkey,IMonkeyObserver * mo);
void monkey_detach_observer(Monkey * monkey,IMonkeyObserver * mo);

#define TYPE_IMONKEY_OBSERVER        (imonkey_observer_get_type())

#define IMONKEY_OBSERVER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_IMONKEY_OBSERVER,IMonkeyObserver))
#define IS_IMONKEY_OBSERVER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_IMONKEY_OBSERVER))

#define IMONKEY_OBSERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_IMONKEY_OBSERVER, IMonkeyObserverClass))
#define IMONKEY_OBSERVER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TYPE_IMONKEY_OBSERVER, IMonkeyObserverClass))

typedef struct IMonkeyObserverClassPrivate IMonkeyObserverClassPrivate;
typedef struct {
  GTypeInterface root_interface;
  IMonkeyObserverClassPrivate * private;
} IMonkeyObserverClass;


GType imonkey_observer_get_type(void);

void imonkey_observer_class_virtual_init(IMonkeyObserverClass * i,
													  void (*  game_lost)(IMonkeyObserver *bo,
																				 Monkey * monkey),
													  void (* bubbles_exploded)(IMonkeyObserver * bo,
																						 Monkey * monkey,
																						 GList * exploded,
																						 GList * fallen),
													  void (*bubble_shot)(IMonkeyObserver * bo,
																				 Monkey * monkey,
																				 Bubble * bubble),
													  void (*bubble_sticked)(IMonkeyObserver * bo,
																					 Monkey * monkey,
																					 Bubble * bubble),
													  void (*bubbles_waiting_changed)(IMonkeyObserver * bo,
																								Monkey * monkey,
																								int bubbles_count));
void imonkey_observer_game_lost(IMonkeyObserver * bo,
										  Monkey * monkey);


void imonkey_observer_bubble_shot(IMonkeyObserver * bo,
											 Monkey * monkey,
											 Bubble * bubble);

void imonkey_observer_bubble_sticked(IMonkeyObserver * bo,
												 Monkey * monkey,
												 Bubble * bubble);

void imonkey_observer_bubbles_waiting_changed(IMonkeyObserver * bo,
															 Monkey * monkey,
															 int bubbles_count);

void imonkey_observer_bubbles_exploded(IMonkeyObserver * bo,
				       Monkey * monkey,
				       GList * exploded,
				       GList * fallen);

G_END_DECLS





#endif
