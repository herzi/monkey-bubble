/* ui-main.h
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
#ifndef __UI_MAIN_H__
#define __UI_MAIN_H__

#include <gtk/gtk.h>
#include "monkey-canvas.h"
#include "game.h"

#define UI_TYPE_MAIN                 (ui_main_get_type ())
#define UI_MAIN(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), UI_TYPE_MAIN, UiMain))
#define UI_MAIN_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), UI_TYPE_MAIN, UiMainClass))
#define UI_IS_MAIN(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UI_TYPE_MAIN))
#define UI_IS_MAIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), UI_TYPE_MAIN))
#define UI_MAIN_GET_CLASS(obj)       (G_TYPE_CHECK_GET_CLASS ((obj), UI_TYPE_MAIN, UiMainClass))

typedef struct UiMainPrivate UiMainPrivate;


typedef struct _UiMain       UiMain;
typedef struct _UiMainClass  UiMainClass;

struct _UiMain
{
  GObject parent_instance;
  UiMainPrivate * private;
};

struct _UiMainClass {
  GObjectClass        parent_class;
};


GType        ui_main_get_type          (void);
UiMain * ui_main_get_instance();
MonkeyCanvas * ui_main_get_canvas(UiMain * ui_main);
GtkWidget * ui_main_get_window(UiMain * ui_main);
Block * ui_main_get_main_image(UiMain * ui_main);
void ui_main_set_game(UiMain *ui_main, Game *game);
void ui_main_enabled_games_item(UiMain * ui_main ,gboolean enabled);
#endif /* __UI_MAIN_H__ */
