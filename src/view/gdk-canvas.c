/* gdk-canvas.c
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
#include "gdk-canvas.h"
#include <librsvg/rsvg.h>
#include <locale.h>

#define PRIVATE(gdk_canvas) (gdk_canvas->private)

static GtkDrawingAreaClass* parent_class = NULL;

struct GdkCanvasPrivate {
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
static void gdk_canvas_scale_images(GdkCanvas * canvas);

void create_pixbuf_svg(Image * i );
void create_pixbuf_normal(Image * i );
void  gdk_canvas_scale_image_ghfunc(gpointer key,
				    gpointer value,
				    gpointer user_data);



gboolean gdk_canvas_configure(GtkWidget * widget,
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

Layer * gdk_canvas_new_layer(gdouble x,gdouble y);

Block * gdk_canvas_create_block(Image * image ,
				gdouble x_center,
				gdouble y_center);

void image_create_pixbuf(Image * i,GdkCanvas * canvas);
Image * gdk_canvas_load_image_from_path( GdkCanvas * canvas,
					 const char * path,
					 gint x_size,
					 gint y_size);

static void gdk_canvas_instance_init(GdkCanvas * gdk_canvas) {
  gdk_canvas->private =g_new0 (GdkCanvasPrivate, 1);			
}

static void gdk_canvas_finalize(GObject* object) {
  GdkCanvas * gdk_canvas = GDK_CANVAS(object);

  // free layer
  g_free(gdk_canvas->private);

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void gdk_canvas_class_init (GdkCanvasClass *klass) {
  GObjectClass* object_class;
    
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = gdk_canvas_finalize;
}


GType gdk_canvas_get_type(void) {
  static GType gdk_canvas_type = 0;
    
  if (!gdk_canvas_type) {
    static const GTypeInfo gdk_canvas_info = {
      sizeof(GdkCanvasClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) gdk_canvas_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(GdkCanvas),
      1,              /* n_preallocs */
      (GInstanceInitFunc) gdk_canvas_instance_init,
    };


      
    gdk_canvas_type = g_type_register_static(gtk_drawing_area_get_type(),
					     "GdkCanvas",
					     &gdk_canvas_info,
					     0);
  }
    
  return gdk_canvas_type;
}

gint gdk_canvas_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data);


GdkCanvas * gdk_canvas_new( void ) {
  GdkCanvas * gdk_canvas;
  GdkCanvasPrivate * dp= NULL;
  
  
  gdk_canvas = GDK_CANVAS (g_object_new (TYPE_GDK_CANVAS, NULL));

  dp = PRIVATE(gdk_canvas);
  dp->img_list = NULL;

  dp->layer_list = NULL;

  dp->region = gdk_region_new();
  dp->layer_list = g_list_append(NULL,
				 gdk_canvas_new_layer(0,0));


  dp->images_map =
    g_hash_table_new(g_str_hash,g_str_equal); 


  dp->x_size= 640;
  dp->y_size = 480;

  dp->real_x_size= 640;
  dp->real_y_size = 480;

  dp->scale_x= 1;
  dp->scale_y = 1;

  gtk_drawing_area_size(GTK_DRAWING_AREA(gdk_canvas),640,480);

  g_signal_connect (GTK_OBJECT (gdk_canvas), "expose-event",
		    GTK_SIGNAL_FUNC (gdk_canvas_expose), gdk_canvas);

  g_signal_connect( GTK_OBJECT(gdk_canvas), "configure-event",
		    GTK_SIGNAL_FUNC (gdk_canvas_configure),gdk_canvas);
  

  dp->buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
			       dp->x_size,dp->y_size);

  return gdk_canvas;
}


void block_get_position(Block * block,
			gdouble *x,
			gdouble *y) {
    
  *x =block->x - block->layer->x + block->x_center;
  *y = block->y - block->layer->y + block->y_center;
}


gboolean gdk_canvas_configure(GtkWidget * widget,
			      GdkEventConfigure *event,
			      gpointer user_data) {

  GdkCanvas * gdk_canvas;
  GdkRectangle rect;
  g_assert( IS_GDK_CANVAS(user_data));
  gdk_canvas = GDK_CANVAS(user_data);
    
    
  PRIVATE(gdk_canvas)->real_x_size = event->width;
  PRIVATE(gdk_canvas)->real_y_size = event->height;

  g_object_unref(PRIVATE(gdk_canvas)->buffer);

  PRIVATE(gdk_canvas)->buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
						PRIVATE(gdk_canvas)->real_x_size,
						PRIVATE(gdk_canvas)->real_y_size);

  PRIVATE(gdk_canvas)->scale_x = 
    (gdouble)PRIVATE(gdk_canvas)->real_x_size /
    (gdouble)PRIVATE(gdk_canvas)->x_size;

  PRIVATE(gdk_canvas)->scale_y = 
    (gdouble)PRIVATE(gdk_canvas)->real_y_size /
    (gdouble)PRIVATE(gdk_canvas)->y_size;

  gdk_canvas_scale_images(gdk_canvas);

  rect.x =0;
  rect.y =0;
  rect.width =     PRIVATE(gdk_canvas)->real_x_size;
  rect.height =     PRIVATE(gdk_canvas)->real_y_size;
    
  gdk_region_union_with_rect( PRIVATE(gdk_canvas)->region,
			      &rect);
  return TRUE;
}


void  gdk_canvas_scale_image_ghfunc(gpointer key,
				    gpointer value,
				    gpointer user_data) {

  GdkCanvas * canvas;
  Image * i;

  canvas = GDK_CANVAS(user_data);
  i = (Image * )value;

  i->scale_x = PRIVATE(canvas)->scale_x;
  i->scale_y = PRIVATE(canvas)->scale_y;

  i->scaled_width = (gdouble)i->x_size * i->scale_x;
  i->scaled_height = (gdouble)i->y_size * i->scale_y;


  image_create_pixbuf(i,canvas);

}

static void gdk_canvas_scale_images(GdkCanvas * canvas) {
  g_hash_table_foreach( PRIVATE(canvas)->images_map,
			&gdk_canvas_scale_image_ghfunc,
			canvas);

}

gint gdk_canvas_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  guchar *pixels;
  GdkCanvas * gdk_canvas;
  int rowstride;

  gdk_canvas = GDK_CANVAS(data);
  
  rowstride = gdk_pixbuf_get_rowstride (PRIVATE(gdk_canvas)->buffer);
  pixels = gdk_pixbuf_get_pixels (PRIVATE(gdk_canvas)->buffer) + rowstride * event->area.y + event->area.x * 4;
  gdk_draw_rgb_32_image_dithalign (widget->window,
				   widget->style->black_gc,
				   event->area.x, event->area.y,
				   event->area.width, event->area.height,
				   GDK_RGB_DITHER_NONE,
				   pixels, rowstride,
				   event->area.x, event->area.y);

  return TRUE;
}

void gdk_canvas_draw(GdkCanvas * di) {
  
}

Block * gdk_canvas_create_block_from_image(GdkCanvas * canvas,
					   const char * path,
					   gint x_size,
					   gint y_size,
					   gint x_center,
					   gint y_center) {

  Image * image;
  g_assert( IS_GDK_CANVAS( canvas) );


  image = g_hash_table_lookup( PRIVATE(canvas)->images_map,
			       path);


  if( image == NULL ) {

		  
    image = gdk_canvas_load_image_from_path( canvas,
					     path ,
					     x_size,
					     y_size);
    
		  
    g_hash_table_insert( PRIVATE(canvas)->images_map,
			 image->path,
			 image);
		  
  }

  
  return gdk_canvas_create_block( image,x_center,y_center );
}

Block * gdk_canvas_create_block(Image * image,
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

Image * gdk_canvas_load_image_from_path( GdkCanvas * canvas,
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

void image_scale(Image * i,GdkCanvas * canvas) {
    
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
    
  i->scaled_width = (gdouble)i->scale_x*i->x_size;
  i->scaled_height = (gdouble)i->scale_y*i->y_size;

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



  i->scaled_width = (gdouble)i->scale_x*i->x_size;
  i->scaled_height = (gdouble)i->scale_y*i->y_size;

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

void image_create_pixbuf(Image * i,GdkCanvas * canvas) {
    
  i->create_pixbuf(i);

}

Layer * gdk_canvas_get_root_layer(GdkCanvas * canvas) {

  g_assert( IS_GDK_CANVAS( canvas ));

  return (Layer *) PRIVATE(canvas)->layer_list->data;
}

Layer * gdk_canvas_append_layer(GdkCanvas * canvas,
				gdouble x,
				gdouble y) {
  
  
  Layer * l;
  
  g_assert( IS_GDK_CANVAS( canvas ) );

  l = gdk_canvas_new_layer(x,y);
  PRIVATE(canvas)->layer_list = 
    g_list_append( PRIVATE( canvas )->layer_list,
		   l);
  
  return l;
}

Layer * gdk_canvas_new_layer(gdouble x,
			     gdouble y) {
  Layer * l;
  l = g_malloc( sizeof(Layer) );
  l->block_list = NULL;
  l->x = x;
  l->y = y;
  return l;
}

void gdk_canvas_add_block(GdkCanvas * canvas,
			  Layer * layer,
			  Block * block,
			  gdouble x,
			  gdouble y) {

  GdkRectangle rect;
  g_assert( IS_GDK_CANVAS( canvas ) );

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
void gdk_canvas_move_block(GdkCanvas * canvas,
			   Block * block,
			   gdouble x,
			   gdouble y)
{
  GdkRectangle rect;

  g_assert( IS_GDK_CANVAS( canvas ) );

  block_get_rectangle(block,&rect);


  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);

  block_set_position(block,x,y);
  block_get_rectangle(block,&rect);

  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);

}

void gdk_canvas_remove_block(GdkCanvas * canvas,
			     Block * block){

  Layer * l;
  GdkRectangle rect;
  g_assert( IS_GDK_CANVAS( canvas ));
  
  l = block->layer;

  l->block_list = g_list_remove( l->block_list,
				 block);

  block_get_rectangle(block,&rect);

  gdk_region_union_with_rect( PRIVATE(canvas)->region,
			      &rect);
  
}


void gdk_canvas_unref_block(GdkCanvas * canvas,
			    Block * b) {
  g_assert(IS_GDK_CANVAS(canvas));

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
    
    g_free( next->data);
    next = g_list_next( next);	
  }

  g_list_free(l->block_list);

  g_free(l);
}



void gdk_canvas_clear(GdkCanvas * gdk_canvas){

  GList * next;
  g_assert( IS_GDK_CANVAS(gdk_canvas));

  next = PRIVATE(gdk_canvas)->layer_list;
  while( next != NULL ) {
    layer_delete((Layer * )next->data);
    next = g_list_next(next);
  }

  g_list_free( PRIVATE(gdk_canvas)->layer_list);

  PRIVATE(gdk_canvas)->layer_list = g_list_append(NULL,
						  gdk_canvas_new_layer(0,0));

}

void gdk_canvas_paint(GdkCanvas * gdk_canvas){

  int rect_count;
  int i;
  
  GdkRectangle * rects;
  
  GdkRegion * screen;

  GdkRectangle screen_rect;
  

  GList * next;

  g_assert(IS_GDK_CANVAS(  gdk_canvas ) );
  
  
  screen_rect.x=0;
  screen_rect.y=0;
  screen_rect.width= PRIVATE(gdk_canvas)->real_x_size;
  screen_rect.height=PRIVATE(gdk_canvas)->real_y_size;

  screen = gdk_region_new();
  gdk_region_union_with_rect(screen,&screen_rect);
  gdk_region_intersect( PRIVATE(gdk_canvas)->region,screen );
  
  
  if(! gdk_region_empty( PRIVATE(gdk_canvas)->region ) ) {
    

    gdk_region_get_rectangles(PRIVATE(gdk_canvas)->region,&rects,&rect_count);
    

    for(i=0;i< rect_count;i++) {

      next = PRIVATE(gdk_canvas)->layer_list;
      while( next != NULL ) {
	    	    
	layer_draw( (Layer*) next->data,
		    PRIVATE(gdk_canvas)->buffer,
		    &rects[i] );
	next = g_list_next( next );
      }
	
      //	gtk_widget_draw (GTK_WIDGET(gdk_canvas), &rects[i]);
      gtk_widget_queue_draw_area(GTK_WIDGET(gdk_canvas),rects[i].x,rects[i].y,
				 rects[i].width,rects[i].height);
    }
    
    //	 gtk_widget_queue_draw(GTK_WIDGET(gdk_canvas));
    if( rects != NULL) g_free(rects);
    
    gdk_region_destroy( PRIVATE(gdk_canvas)->region);
    gdk_region_destroy( screen);
    
    PRIVATE(gdk_canvas)->region = gdk_region_new();
  }
  
  
}
