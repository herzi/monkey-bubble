/* keyboard-propeties.h
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
#ifndef __KEYBOARD_PROPERTIES_H__
#define __KEYBOARD_PROPERTIES_H__

#include <gtk/gtk.h>

#define CONF_KEY_PREFIX "/apps/monkey-bubble"
#define CONF_GLOBAL_PREFIX "/apps/monkey-bubble"

#define ACCEL_PATH_ROOT "<monkey-bubble-accels>/menu"
#define ACCEL_PATH_NEW_1_PLAYER ACCEL_PATH_ROOT"/new_1_player_game"
#define ACCEL_PATH_NEW_2_PLAYERS ACCEL_PATH_ROOT"/new_2_players_game"
#define ACCEL_PATH_PAUSE_GAME ACCEL_PATH_ROOT"/pause_game"
#define ACCEL_PATH_STOP_GAME ACCEL_PATH_ROOT"/stop_game"
#define ACCEL_PATH_QUIT_GAME ACCEL_PATH_ROOT"/quit_game"

#define ACCEL_PATH_FULL_SCREEN ACCEL_PATH_ROOT"/full_screen"
#define ACCEL_PATH_ZOOM_NORMAL ACCEL_PATH_ROOT"/zoom_normal"


#define ACCEL_PATH_PLAYER_1_AIM_LEFT ACCEL_PATH_ROOT"/player_1_left"
#define ACCEL_PATH_PLAYER_1_AIM_RIGHT ACCEL_PATH_ROOT"/player_1_right"
#define ACCEL_PATH_PLAYER_1_SHOOT ACCEL_PATH_ROOT"/player_1_shoot"


#define ACCEL_PATH_PLAYER_2_AIM_LEFT ACCEL_PATH_ROOT"/player_2_left"
#define ACCEL_PATH_PLAYER_2_AIM_RIGHT ACCEL_PATH_ROOT"/player_2_right"
#define ACCEL_PATH_PLAYER_2_SHOOT ACCEL_PATH_ROOT"/player_2_shoot"


#define TYPE_KEYBOARD_PROPERTIES                 (keyboard_properties_get_type ())
#define KEYBOARD_PROPERTIES(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_KEYBOARD_PROPERTIES, KeyboardProperties))
#define KEYBOARD_PROPERTIES_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_KEYBOARD_PROPERTIES, KeyboardPropertiesClass))
#define IS_KEYBOARD_PROPERTIES(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_KEYBOARD_PROPERTIES))
#define IS_KEYBOARD_PROPERTIES_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_KEYBOARD_PROPERTIES))
#define KEYBOARD_PROPERTIES_GET_CLASS(obj)       (G_TYPE_CHECK_GET_CLASS ((obj), TYPE_KEYBOARD_PROPERTIES, KeyboardPropertiesClass))

typedef struct KeyboardPropertiesPrivate KeyboardPropertiesPrivate;


typedef struct _KeyboardProperties       KeyboardProperties;
typedef struct _KeyboardPropertiesClass  KeyboardPropertiesClass;

struct _KeyboardProperties
{
  GObject parent_instance;
  KeyboardPropertiesPrivate * private;
};

struct _KeyboardPropertiesClass {
  GObjectClass        parent_class;
};


GType        keyboard_properties_get_type          (void);

KeyboardProperties * keyboard_properties_get_instance(void);

void keyboard_properties_show(KeyboardProperties * kp,GtkWindow * transient_window);

GtkAccelGroup * keyboard_properties_get_accel_group(KeyboardProperties * kp);
#endif /* __KEYBOARD_PROPERTIES_H__ */
