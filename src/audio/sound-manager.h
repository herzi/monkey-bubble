/* sound-manager.h
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

#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <gtk/gtk.h>
#include "monkey.h"

G_BEGIN_DECLS

#define TYPE_SOUND_MANAGER      (sound_manager_get_type())

#define SOUND_MANAGER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_SOUND_MANAGER,SoundManager))
#define SOUND_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_SOUND_MANAGER,SoundManagerClass))
#define IS_SOUND_MANAGER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_SOUND_MANAGER))
#define IS_SOUND_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_SOUND_MANAGER))
#define SOUND_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_SOUND_MANAGER, SoundManagerClass))



typedef struct SoundManagerPrivate SoundManagerPrivate;



typedef struct {
  GObject parent_instance;
  SoundManagerPrivate * private;
} SoundManager ;

typedef struct {
  GObjectClass parent_class;
} SoundManagerClass;


GType sound_manager_get_type(void);

SoundManager * sound_manager_new();

void sound_manager_active_sound(gboolean active);
void sound_manager_play_music_file(SoundManager *m, gchar * path);
//void sound_manager_update(SoundManager *m);
G_END_DECLS





#endif
