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
void keyboard_properties_show_instance();
#endif /* __KEYBOARD_PROPERTIES_H__ */
