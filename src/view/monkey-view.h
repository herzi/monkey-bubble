/* monkey-view.h
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

#ifndef MONKEY_VIEW_H
#define MONKEY_VIEW_H

#include <gtk/gtk.h>
#include "color.h"
#include "monkey-canvas.h"
#include "monkey.h"

G_BEGIN_DECLS

#define TYPE_MONKEY_VIEW            (monkey_view_get_type())

#define MONKEY_VIEW(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_MONKEY_VIEW,MonkeyView))
#define MONKEY_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MONKEY_VIEW,MonkeyViewClass))
#define IS_MONKEY_VIEW(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_MONKEY_VIEW))
#define IS_MONKEY_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MONKEY_VIEW))
#define MONKEY_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MONKEY_VIEW, MonkeyViewClass))

typedef struct MonkeyViewPrivate MonkeyViewPrivate;

typedef struct {
  GObject parent_instance;
  MonkeyViewPrivate * private;
} MonkeyView;

typedef struct {
  GObjectClass parent_class;
} MonkeyViewClass;


GType monkey_view_get_type(void);

MonkeyView * monkey_view_new(MonkeyCanvas * canvas,Monkey * monkey,
		       gint x,gint y,gboolean back_needed);

void monkey_view_update(MonkeyView * monkey_view,
		     gint time );

void monkey_view_set_gems_count(MonkeyView * d,int gems);
void monkey_view_set_score(MonkeyView * d,int score);
void monkey_view_set_points(MonkeyView * d,int points);
void monkey_view_set_waiting_bubbles(MonkeyView * d,int bubbles);


void monkey_view_draw_lost(MonkeyView *d);
void monkey_view_draw_win(MonkeyView *d);

G_END_DECLS
#endif
