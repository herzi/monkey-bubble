/* monkey-canvas.c
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
#include "monkey-canvas.h"
#include <librsvg/rsvg.h>
#include <locale.h>
#include <math.h>
#define PRIVATE(monkey_canvas) (monkey_canvas->private)

static GtkDrawingAreaClass* parent_class = NULL;

struct MonkeyCanvasPrivate {
  GList * img_list;
  GList * layer_list;
  GdkPixbuf * buffer;
  GHashTable * images_map;
  GdkRegion * region;
  int x_size;
  int y_size;
  int real_x_size;
  int real_y_size;
  gdouble scale_x;
  gdouble scale_y;    
};

typedef struct Image Image;
struct Image {
  GdkPixbuf * scaled_pixbuf;
  GdkPixbuf * original_pix;
  GdkRectangle rect;
  char * path;
  gint x_size;
  gint y_size;

  gdouble scale_x;
  gdouble scale_y;

  gint scaled_width;
  gint scaled_height;
  void (* create_pixbuf) (Image * image);
} ;

struct Layer {
  GList * block_list;
  gdouble x;
  gdouble y;
};

struct Block {
  Image * image;
  Layer * layer;
  gdouble x;
  gdouble y;
  gdouble x_center;
  gdouble y_center;
};
static void monkey_canvas_scale_images(MonkeyCanvas * canvas);

void create_pixbuf_svg(Image * i );
void create_pixbuf_normal(Image * i );
void  monkey_canvas_scale_image_ghfunc(gpointer key,
				    gpointer value,
				    gpointer user_data);



gboolean monkey_canvas_configure(GtkWidget * widget,
			      GdkEventConfigure *event,
			      gpointer user_data);
void block_draw(Block * block,
		GdkPixbuf * pixbuf,
		GdkRectangle * rect);
void layer_draw(Layer* l,
		GdkPixbuf * pixbuf,
		GdkRectangle * rect);

void block_set_position(Block * block,
			gdouble x,
			gdouble y);

void block_get_rectangle(Block * block,
			 GdkRectangle * rect);

Layer * monkey_canvas_new_layer(gdouble x,gdouble y);

Block * monkey_canvas_create_block(Image * image ,
				gdouble x_center,
				gdouble y_center);

void image_create_pixbuf(Image * i,MonkeyCanvas * canvas);
Image * monkey_canvas_load_image_from_path( MonkeyCanvas * canvas,
					 const char * path,
					 gint x_size,
					 gint y_size);

static void monkey_canvas_instance_init(MonkeyCanvas * monkey_canvas) {
  monkey_canvas->private =g_new0 (MonkeyCanvasPrivate, 1);			
}

static void monkey_canvas_finalize(GObject* object) {
  MonkeyCanvas * monkey_canvas = MONKEY_CANVAS(object);

  // free layer
  g_free(monkey_canvas->private);

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void monkey_canvas_class_init (MonkeyCanvasClass *klass) {
  GObjectClass* object_class;
    
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = monkey_canvas_finalize;
}


GType monkey_canvas_get_type(void) {
  static GType monkey_canvas_type = 0;
    
  if (!monkey_canvas_type) {
    static const GTypeInfo monkey_canvas_info = {
      sizeof(MonkeyCanvasClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) monkey_canvas_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(MonkeyCanvas),
      1,              /* n_preallocs */
      (GInstanceInitFunc) monkey_canvas_instance_init,
    };


      
    monkey_canvas_type = g_type_register_static(gtk_drawing_area_get_type(),
					     "MonkeyCanvas",
					     &monkey_canvas_info,
					     0);
  }
    
  return monkey_canvas_type;
}

gint monkey_canvas_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data);


MonkeyCanvas * monkey_canvas_new( void ) {
  MonkeyCanvas * monkey_canvas;
  MonkeyCanvasPrivate * dp= NULL;
  
  
  monkey_canvas = MONKEY_CANVAS (g_object_new (TYPE_MONKEY_CANVAS, NULL));

  dp = PRIVATE(monkey_canvas);
  dp->img_list = NULL;

  dp->layer_list = NULL;

  dp->region = gdk_region_new();
  dp->layer_list = g_list_append(NULL,
				 monkey_canvas_new_layer(0,0));


  dp->images_map =
    g_hash_table_new(g_str_hash,g_str_equal); 


  dp->x_size= 640;
  dp->y_size = 480;

  dp->real_x_size= 640;
  dp->real_y_size = 480;

  dp->scale_x= 1;
  dp->scale_y = 1;

  gtk_drawing_area_size(GTK_DRAWING_AREA(monkey_canvas),640,480);

  g_signal_connect (GTK_OBJECT (monkey_canvas), "expose-event",
		    GTK_SIGNAL_FUNC (monkey_canvas_expose), monkey_canvas);

  g_signal_connect( GTK_OBJECT(monkey_canvas), "configure-event",
		    GTK_SIGNAL_FUNC (monkey_canvas_configure),monkey_canvas);
  

  dp->buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
			       dp->x_size,dp->y_size);

  return monkey_canvas;
}


void block_get_position(Block * block,
			gdouble *x,
			gdouble *y) {
    
  *x =block->x - block->layer->x + block->x_center;
  *y = block->y - block->layer->y + block->y_center;
}


gboolean monkey_canvas_configure(GtkWidget * widget,
			      GdkEventConfigure *event,
			      gpointer user_data) {

  MonkeyCanvas * monkey_canvas;
  GdkRectangle rect;
  g_assert( IS_MONKEY_CANVAS(user_data));
  monkey_canvas = MONKEY_CANVAS(user_data);
    
    
  PRIVATE(monkey_canvas)->real_x_size = event->width;
  PRIVATE(monkey_canvas)->real_y_size = event->height;

  g_object_unref(PRIVATE(monkey_canvas)->buffer);

  PRIVATE(monkey_canvas)->buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
						PRIVATE(monkey_canvas)->real_x_size,
						PRIVATE(monkey_canvas)->real_y_size);

  PRIVATE(monkey_canvas)->scale_x = 
    (gdouble)PRIVATE(monkey_canvas)->real_x_size /
    (gdouble)PRIVATE(monkey_canvas)->x_size;

  PRIVATE(monkey_canvas)->scale_y = 
    (gdouble)PRIVATE(monkey_canvas)->real_y_size /
    (gdouble)PRIVATE(monkey_canvas)->y_size;

  monkey_canvas_scale_images(monkey_canvas);

  rect.x =0;
  rect.y =0;
  rect.width =     PRIVATE(monkey_canvas)->real_x_size;
  rect.height =     PRIVATE(monkey_canvas)->real_y_size;
    
  gdk_region_union_with_rect( PRIVATE(monkey_canvas)->region,
			      &rect);

  monkey_canvas_paint( monkey_canvas);
  return TRUE;
}


void  monkey_canvas_scale_image_ghfunc(gpointer key,
				    gpointer value,
				    gpointer user_data) {

  MonkeyCanvas * canvas;
  Image * i;

  canvas = MONKEY_CANVAS(user_data);
  i = (Image * )value;

  i->scale_x = PRIVATE(canvas)->scale_x;
  i->scale_y = PRIVATE(canvas)->scale_y;

  i->scaled_width = rint((gdouble)i->x_size * i->scale_x);
  i->scaled_height = rint((gdouble)i->y_size * i->scale_y);


  image_create_pixbuf(i,canvas);

}

static void monkey_canvas_scale_images(MonkeyCanvas * canvas) {
  g_hash_table_foreach( PRIVATE(canvas)->images_map,
			&monkey_canvas_scale_image_ghfunc,
			canvas);

}

gint monkey_canvas_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  guchar *pixels;
  MonkeyCanvas * monkey_canvas;
  int rowstride;

  monkey_canvas = MONKEY_CANVAS(data);
  
  rowstride = gdk_pixbuf_get_rowstride (PRIVATE(monkey_canvas)->buffer);
  pixels = gdk_pixbuf_get_pixels (PRIVATE(monkey_canvas)->buffer) + rowstride * event->area.y + event->area.x * 4;
  gdk_draw_rgb_32_image_dithalign (widget->window,
				   widget->style->black_gc,
				   event->area.x, event->area.y,
				   event->area.width, event->area.height,
				   GDK_RGB_DITHER_NONE,
				   pixels, rowstride,
				   event->area.x, event->area.y);
  return TRUE;
}

void monkey_canvas_draw(MonkeyCanvas * di) {
  
}

Block * monkey_canvas_create_block_from_image(MonkeyCanvas * canvas,
					   const char * path,
					   gint x_size,
					   gint y_size,
					   gint x_center,
					   gint y_center) {

  Image * image;
  g_assert( IS_MONKEY_CANVAS( canvas) );


  image = g_hash_table_lookup( PRIVATE(canvas)->images_map,
			       path);


  if( image == NULL ) {

		  
    image = monkey_canvas_load_image_from_path( canvas,
					     path ,
					     x_size,
					     y_size);
    
		  
    g_hash_table_insert( PRIVATE(canvas)->images_map,
			 image->path,
			 image);
		  
  }

  
  return monkey_canvas_create_block( image,x_center,y_center );
}

Block * monkey_canvas_create_block(Image * image,
				gdouble x_center,
				gdouble y_center) {

  Block * b;

  b = g_malloc( sizeof(Block));
  b->image = image;
  b->layer = NULL;
  b->x = 0;
  b->y = 0;
  b->x_center = x_center;
  b->y_center = y_center;
    
  return b;
}

Image * monkey_canvas_load_image_from_path( MonkeyCanvas * canvas,
					 const char * path,
					 gint x_size,
					 gint y_size) {

  Image * i;
  gchar ** c;
  gchar * cc;
  int ii;
  i = g_malloc( sizeof( Image ));

  i->x_size = x_size;
  i->y_size = y_size;
  i->path = g_strdup( path);

  i->scaled_pixbuf = NULL;
  i->original_pix = NULL;


  // TODO : clean this
  c = g_strsplit(path,
		 ".svg",
		 0);
    
  ii=0;
  cc = c[ii];
    
  while( cc!= NULL ) {
    ii++;
    cc = c[ii];
  }
	 
  g_strfreev(c);
  // end of the clean
  if( ii > 1 ) {
    i->create_pixbuf = &create_pixbuf_svg;
  } else {
    i->create_pixbuf = &create_pixbuf_normal;
  }




  i->scale_x = PRIVATE(canvas)->scale_x;
  i->scale_y = PRIVATE(canvas)->scale_y;

  image_create_pixbuf(i,canvas);

  return i;
}

void image_scale(Image * i,MonkeyCanvas * canvas) {
    
  i->scale_x = PRIVATE(canvas)->scale_x;
  i->scale_y = PRIVATE(canvas)->scale_y;
  image_create_pixbuf( i,canvas);
}


void create_pixbuf_normal(Image * i ) {

  GError * error;
  error = NULL;

  if( i->scaled_pixbuf != NULL ) {
    g_object_unref( i->scaled_pixbuf);
  }


  if( i->original_pix == NULL ) {
    i->original_pix = gdk_pixbuf_new_from_file(i->path, 
					       &error); 
    if( error != NULL ) {
      g_error("Erreur load image");
    }
    if( i->x_size == -1 ) {
      i->x_size = gdk_pixbuf_get_width( i->original_pix );
    }
	
    if( i->y_size == - 1) {
      i->y_size = gdk_pixbuf_get_height( i->original_pix );
    }
	
  }
    
  i->scaled_width = ceil(i->scale_x*(gdouble)i->x_size);
  i->scaled_height = ceil(i->scale_y*(gdouble)i->y_size);

  i->scaled_pixbuf = gdk_pixbuf_scale_simple( i->original_pix,
					      i->scaled_width,
					      i->scaled_height,
					      GDK_INTERP_BILINEAR);

}


void create_pixbuf_svg(Image * i ) {
  char *num_locale;
  GError * error;
  error = NULL;

  if( i->scaled_pixbuf != NULL ) {
    g_object_unref( i->scaled_pixbuf);
  }



  i->scaled_width = ceil( i->scale_x* (gdouble) i->x_size );
  i->scaled_height = ceil(i->scale_y*(gdouble)i->y_size);

    num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
    setlocale (LC_NUMERIC, "C");

  i->scaled_pixbuf = rsvg_pixbuf_from_file_at_size   (i->path,
						      i->scaled_width,
						      i->scaled_height,
						      &error);
    
    setlocale (LC_NUMERIC, num_locale);
    g_free (num_locale);
    num_locale = NULL;

  if( error != NULL ) {
    g_error("Erreur load scg");
  }
}

void image_create_pixbuf(Image * i,MonkeyCanvas * canvas) {
    
  i->create_pixbuf(i);

}

Layer * monkey_canvas_get_root_layer(MonkeyCanvas * canvas) {

  g_assert( IS_MONKEY_CANVAS( canvas ));

  return (Layer *) PRIVATE(canvas)->layer_list->data;
}

Layer * monkey_canvas_append_layer(MonkeyCanvas * canvas,
				gdouble x,
				gdouble y) {
  
  
  Layer * l;
  
  g_assert( IS_MONKEY_CANVAS( canvas ) );

  l = monkey_canvas_new_layer(x,y);
  PRIVATE(canvas)->layer_list = 
    g_list_append( PRIVATE( canvas )->layer_list,
		   l);
  
  return l;
}

Layer * monkey_canvas_new_layer(gdouble x,
			     gdouble y) {
  Layer * l;
  l = g_malloc( sizeof(Layer) );
  l->block_list = NULL;
  l->x = x;
  l->y = y;
  return l;
}

void monkey_canvas_add_block(MonkeyCanvas * canvas,
			  Layer * layer,
			  Block * block,
			  gdouble x,
			  gdouble y) {

  GdkRectangle rect;
  g_assert( IS_MONKEY_CANVAS( canvas ) );


  layer->block_list = g_list_append( layer->block_list,
				     block);

  block->layer = layer;

    
  block_set_position(block,x,y);

  block_get_rectangle(block,&rect);

  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);
			
}

void block_set_position(Block * block,
			gdouble x,
			gdouble y) {

  block->x =x + block->layer->x - block->x_center;
  block->y = y + block->layer->y - block->y_center;
    
}

void block_get_rectangle(Block * block,
			 GdkRectangle * rect) {

  rect->x = block->x * block->image->scale_x;
  rect->y = block->y * block->image->scale_y;

  rect->width = block->image->scaled_width;
  rect->height = block->image->scaled_height;


}
void monkey_canvas_move_block(MonkeyCanvas * canvas,
			   Block * block,
			   gdouble x,
			   gdouble y)
{
  GdkRectangle rect;

  g_assert( IS_MONKEY_CANVAS( canvas ) );

  block_get_rectangle(block,&rect);


  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);

  block_set_position(block,x,y);
  block_get_rectangle(block,&rect);

  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);

}

void monkey_canvas_remove_block(MonkeyCanvas * canvas,
			     Block * block){

  Layer * l;
  GdkRectangle rect;
  g_assert( IS_MONKEY_CANVAS( canvas ));
  
  l = block->layer;

  l->block_list = g_list_remove( l->block_list,
				 block);

  block_get_rectangle(block,&rect);

  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);
  
}


void monkey_canvas_unref_block(MonkeyCanvas * canvas,
			    Block * b) {
  g_assert(IS_MONKEY_CANVAS(canvas));

  g_free(b);
}

void layer_draw( Layer* l,
		 GdkPixbuf * pixbuf,
		 GdkRectangle * rect) {

  GList * next;
    
  next = l->block_list;
  while( next != NULL ) {

    block_draw( (Block *) next->data,
		pixbuf,
		rect  );
    next = g_list_next( next );    
	
  }

}

void block_draw(Block * block,
		GdkPixbuf * buf,
		GdkRectangle * rect) {

  GdkRectangle irect;
  GdkRectangle r;
    
  block_get_rectangle(block,&r);


  if( gdk_rectangle_intersect( &r,rect,&irect)){

    gdk_pixbuf_composite(block->image->scaled_pixbuf, buf,
			 irect.x,irect.y,
			 irect.width,irect.height,
			 r.x,
			 r.y,
			 1,1,
			 GDK_INTERP_NEAREST,
			 255);
    
    
  }

}

static void layer_delete(Layer * l) {
  GList * next;
  next = l->block_list;
  while( next != NULL) {
    
    next = g_list_next( next);	
  }

  g_list_free(l->block_list);

  g_free(l);
}



void monkey_canvas_clear(MonkeyCanvas * monkey_canvas){

  GList * next;
  g_assert( IS_MONKEY_CANVAS(monkey_canvas));

  next = PRIVATE(monkey_canvas)->layer_list;
  while( next != NULL ) {
    layer_delete((Layer * )next->data);
    next = g_list_next(next);
  }

  g_list_free( PRIVATE(monkey_canvas)->layer_list);

  PRIVATE(monkey_canvas)->layer_list = g_list_append(NULL,
						  monkey_canvas_new_layer(0,0));

}

void monkey_canvas_paint(MonkeyCanvas * monkey_canvas){

  int rect_count;
  int i;
  
  GdkRectangle * rects;
  
  GdkRegion * screen;

  GdkRectangle screen_rect;
  

  GList * next;

  g_assert(IS_MONKEY_CANVAS(  monkey_canvas ) );
  
  
  screen_rect.x=0;
  screen_rect.y=0;
  screen_rect.width= PRIVATE(monkey_canvas)->real_x_size;
  screen_rect.height=PRIVATE(monkey_canvas)->real_y_size;

  screen = gdk_region_new();
  gdk_region_union_with_rect(screen,&screen_rect);
  gdk_region_intersect( PRIVATE(monkey_canvas)->region,screen );
  
  
  if(! gdk_region_empty( PRIVATE(monkey_canvas)->region ) ) {
    

    gdk_region_get_rectangles(PRIVATE(monkey_canvas)->region,&rects,&rect_count);
    

    for(i=0;i< rect_count;i++) {

      next = PRIVATE(monkey_canvas)->layer_list;
      while( next != NULL ) {
	    	    
	layer_draw( (Layer*) next->data,
		    PRIVATE(monkey_canvas)->buffer,
		    &rects[i] );
	next = g_list_next( next );
      }
	
      //	gtk_widget_draw (GTK_WIDGET(monkey_canvas), &rects[i]);
      gtk_widget_queue_draw_area(GTK_WIDGET(monkey_canvas),rects[i].x,rects[i].y,
				 rects[i].width,rects[i].height);
    }
    
    //	 gtk_widget_queue_draw(GTK_WIDGET(monkey_canvas));
    if( rects != NULL) g_free(rects);
    
    gdk_region_destroy( PRIVATE(monkey_canvas)->region);
    gdk_region_destroy( screen);
    
    PRIVATE(monkey_canvas)->region = gdk_region_new();
  }
  
  
}
