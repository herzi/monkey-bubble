/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* gdk_view.c
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
#include "gdk-view.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUBBLE_COUNT 8
#define SHOOTER_COUNT 40*2

#define PRIVATE( gdk_view ) (gdk_view->private)
static GObjectClass* parent_class = NULL;

typedef struct Star {
        gdouble vx,vy;
        Block * block;
} Star;

typedef struct BubbleAdded {
        Block * block;
        Bubble * bubble;
    
} BubbleAdded;

struct GdkViewPrivate {
        gint last_time;
        GdkCanvas * canvas;
        Monkey * monkey;
        GList * fallen_list;
        GList * added_list;
        Layer * bubble_layer;
        Layer * shooter_layer;
        Layer * background_layer;
        Layer * ground_layer;
        Layer * monkeys_layer;
        
        Layer * star_layer;

        GHashTable * hash_map;
        Block * shooter_block[SHOOTER_COUNT+1];
        Block * current_shooter_block;
        Block * background; 
        Block * bback;
        Block * win;
        Block * lost;

        Block * snake_body;
        Block * harm;
        Block * monkeys;
        GList * waiting_list;
        GList * score_list;
        GList * points_list;
        GList * gems_list;
        GList * star_list;
        gint time;

};

static void gdk_view_animate_stars(GdkView * d,gint time);
static void gdk_view_add_explode_stars(GdkView * d,Bubble * b) ;
static void gdk_view_bubble_changed(Bubble * b,GdkView * view);

static void gdk_view_bubbles_waiting_changed(Monkey * monkey,
					     int bubbles_count,
					     GdkView * view);

static Block * gdk_view_create_gem(GdkView * d);
static Block * gdk_view_create_big_waiting(GdkView * d);
static Block * gdk_view_create_little_waiting(GdkView * d);
static void gdk_view_free_map(gpointer key,
			      gpointer value,
			      gpointer user_data);

static void gdk_view_shooter_bubble_added(Shooter * shooter,
					  Bubble * b,
					  GdkView * view);

static void gdk_view_shooter_rotated(Shooter * shooter,GdkView * view);

static void gdk_view_bubbles_exploded(    Board * board,
                                          GList * exploed,
                                          GList * fallen,
					  GdkView * view);
static void gdk_view_bubbles_added(  Board * board,
                                     GList * bubbles,
				     GdkView * view);


static void gdk_view_board_down(Board * board,
				GdkView * view);


static void gdk_view_bubbles_inserted(    Board * board,
                                          Bubble ** bubbles,
                                          int count,
					  GdkView * view);




static Block * gdk_view_create_bubble(GdkView * view,
				      Bubble * bubble );
void gdk_view_load_shooter_images(GdkView * gdk_view) {
        GError * error;
        gchar path[4096];
        gint str_length;
        gint i;
        Shooter * shooter;
  
        gdouble x,y;

  
        error = NULL;
        shooter = monkey_get_shooter(PRIVATE(gdk_view)->monkey);

  
  
        strcpy( path,DATADIR"/monkey-bubble/gfx/snake/snake_");
  
        str_length  = strlen(path);
        shooter_get_position( shooter,&x,&y);
  
  
        for( i=0; i < SHOOTER_COUNT/2+1 ; ++i) {

      
		snprintf(path+str_length ,7,"%d.svg",i);    
                
    
                PRIVATE(gdk_view)->shooter_block[i] = 
                        gdk_canvas_create_block_from_image(PRIVATE(gdk_view)->canvas,
                                                           path,120,60,
                                                           60 ,					   
                                                           40 );

        }
  

  
        for( i=SHOOTER_COUNT/2+1; i < SHOOTER_COUNT+1 ; i++) {
      
                snprintf(path+str_length ,8,"-%d.svg",i-SHOOTER_COUNT/2);
      
      
      
                PRIVATE(gdk_view)->shooter_block[i] = 
                        gdk_canvas_create_block_from_image(PRIVATE(gdk_view)->canvas,
                                                           path,120,60,
                                                           60,					   
                                                           40);
        }
      
}

GdkView * gdk_view_new(GdkCanvas * canvas,
		       Monkey * monkey,
		       gint x_pos,gint y_pos,gboolean back_needed) {
        gdouble x,y;
        GError * error;
        Shooter * shooter;
        Playground * pl;
        Board * board;
        Bubble * b;
        Block * block;
        int i,j;

        GdkView * gdk_view = GDK_VIEW (g_object_new (TYPE_GDK_VIEW, NULL));

        error = NULL;
  
        g_assert( IS_GDK_VIEW( gdk_view ) );

        g_object_ref( monkey);
        PRIVATE(gdk_view)->monkey = monkey;


        PRIVATE(gdk_view)->last_time = 0;
        PRIVATE(gdk_view)->canvas = canvas;

        PRIVATE(gdk_view)->added_list = NULL;
  
        PRIVATE(gdk_view)->background_layer = gdk_canvas_get_root_layer( canvas );
        PRIVATE(gdk_view)->ground_layer = gdk_canvas_append_layer(canvas,x_pos,y_pos);
        PRIVATE(gdk_view)->shooter_layer = gdk_canvas_append_layer( canvas ,x_pos,y_pos);
        PRIVATE(gdk_view)->bubble_layer = gdk_canvas_append_layer( canvas,x_pos,y_pos );
        PRIVATE(gdk_view)->monkeys_layer = gdk_canvas_append_layer(canvas,x_pos,y_pos);
        PRIVATE(gdk_view)->star_layer = gdk_canvas_append_layer(canvas,x_pos,y_pos);
  
  
        PRIVATE(gdk_view)->hash_map = 
                g_hash_table_new(g_direct_hash,g_direct_equal);
  
  
  
        gdk_view_load_shooter_images(gdk_view);
  
        if(back_needed ) {
      

                PRIVATE(gdk_view)->background = 
                        gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                            DATADIR"/monkey-bubble/gfx/layout_1_player.svg",
                                                            640,480,0,0);
      
                gdk_canvas_add_block( PRIVATE(gdk_view)->canvas,
                                      PRIVATE(gdk_view)->background_layer,
                                      PRIVATE(gdk_view)->background,
                                      0,0);

	
        } else {
		  
                PRIVATE(gdk_view)->background =   NULL;
        }
	 
        PRIVATE(gdk_view)->bback =  gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                                        DATADIR"/monkey-bubble/gfx/pane.svg",
                                                                        280,80,140,40);
        gdk_canvas_add_block( PRIVATE(gdk_view)->canvas,
                              PRIVATE(gdk_view)->ground_layer,
                              PRIVATE(gdk_view)->bback,
                              320,30);
	 

        PRIVATE(gdk_view)->monkeys =  gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                                          DATADIR"/monkey-bubble/gfx/monkeys.svg",
                                                                          300,160,150,0);


        PRIVATE(gdk_view)->harm =  gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                                       DATADIR"/monkey-bubble/gfx/harm.svg",
                                                                       90,60,0,0);


        gdk_canvas_add_block( PRIVATE(gdk_view)->canvas,
                              PRIVATE(gdk_view)->monkeys_layer,
                              PRIVATE(gdk_view)->harm,
                              310,386);

        gdk_canvas_add_block( PRIVATE(gdk_view)->canvas,
                              PRIVATE(gdk_view)->monkeys_layer,
                              PRIVATE(gdk_view)->monkeys,
                              320,350);





        PRIVATE(gdk_view)->snake_body =  gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                                             DATADIR"/monkey-bubble/gfx/snake-body.svg",
                                                                             60,60,30,20);	 
        PRIVATE(gdk_view)->lost = 
                gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                    DATADIR"/monkey-bubble/gfx/lost.svg",
                                                    200,200,
                                                    100,100);
	 
        PRIVATE(gdk_view)->win = 
                gdk_canvas_create_block_from_image( PRIVATE(gdk_view)->canvas,
                                                    DATADIR"/monkey-bubble/gfx/win.svg",
                                                    200,200,
                                                    100,100);
	 
    
        shooter = monkey_get_shooter(PRIVATE(gdk_view)->monkey);
        shooter_get_position(shooter,&x,&y);
    
        PRIVATE(gdk_view)->current_shooter_block = PRIVATE(gdk_view)->shooter_block[0];
    
        gdk_canvas_add_block( PRIVATE(gdk_view)->canvas,
                              PRIVATE(gdk_view)->shooter_layer,
                              PRIVATE(gdk_view)->shooter_block[0],
                              x,y);

        gdk_canvas_add_block( PRIVATE(gdk_view)->canvas,
                              PRIVATE(gdk_view)->monkeys_layer,
                              PRIVATE(gdk_view)->snake_body,
                              x,y);
    
        if( shooter_get_current_bubble( shooter ) != NULL ) {
                gdk_view_shooter_bubble_added(
                                              shooter,
                                              shooter_get_current_bubble( shooter ), gdk_view);
	
        }
    
    
        if( shooter_get_waiting_bubble( shooter ) != NULL ) {
                gdk_view_shooter_bubble_added(
                                              shooter,
                                              shooter_get_waiting_bubble( shooter ),gdk_view);
		  
        }

	PRIVATE(gdk_view)->star_list = NULL;
        PRIVATE(gdk_view)->score_list = NULL;
        PRIVATE(gdk_view)->gems_list = NULL;
        PRIVATE(gdk_view)->waiting_list = NULL;

        PRIVATE(gdk_view)->fallen_list = NULL;
        PRIVATE(gdk_view)->time = 0;
  
        pl = monkey_get_playground( monkey);

        g_signal_connect( G_OBJECT(monkey),"bubbles-waiting-changed",
                          G_CALLBACK( gdk_view_bubbles_waiting_changed),
                          gdk_view);

  
        g_signal_connect( G_OBJECT(shooter),
                          "rotated",
                          G_CALLBACK( gdk_view_shooter_rotated), 
                          gdk_view);		    

        g_signal_connect( G_OBJECT(shooter),
                          "bubble-added",
                          G_CALLBACK( gdk_view_shooter_bubble_added), 
                          gdk_view);		    

        board = playground_get_board( monkey_get_playground( monkey));
 
        g_signal_connect( G_OBJECT( board),
                          "bubbles-exploded",G_CALLBACK(gdk_view_bubbles_exploded),gdk_view);

 
        g_signal_connect( G_OBJECT( board),
                          "bubbles-added",G_CALLBACK(gdk_view_bubbles_added),gdk_view);

 
        g_signal_connect( G_OBJECT( board),
                          "bubbles-inserted",G_CALLBACK(gdk_view_bubbles_inserted),gdk_view);

 
        g_signal_connect( G_OBJECT( board),
                          "down",G_CALLBACK(gdk_view_board_down),gdk_view);


  
        for( j = 0; j < board_get_row_count( board); j++) {
      
                for( i = 0; i < board_get_column_count( board); i++) {
	  
                        if( ( b = board_get_bubble_at(board,i,j) )!= NULL ) {
                                bubble_get_position(b,&x,&y);
                                block = gdk_view_create_bubble(gdk_view,b);
                                gdk_canvas_add_block(PRIVATE(gdk_view)->canvas,
                                                     PRIVATE(gdk_view)->bubble_layer , 
                                                     block,
                                                     x,y);
	
                                g_hash_table_insert( PRIVATE(gdk_view)->hash_map, b, block );
	
                                g_signal_connect( G_OBJECT(b), "bubble-changed", G_CALLBACK( gdk_view_bubble_changed), gdk_view);
	
	
                        }
      
                }
    
        }
  
        return gdk_view;
}


void gdk_view_update(GdkView * gdk_view,
		     gint time) {
    
        GList * next;
        Block * block;
        gdouble x,y,x2,y2;
        BubbleAdded * added;
        gint dtime;
        g_assert( IS_GDK_VIEW(gdk_view));
    
        next = PRIVATE(gdk_view)->fallen_list;
        
        dtime = time - PRIVATE(gdk_view)->time;
        PRIVATE(gdk_view)->time = time;
        
        while( next != NULL ) {
	
                block = next->data;
                block_get_position( block ,&x,&y);

                if( y > 480) {
                        next = g_list_previous(next);
                        PRIVATE(gdk_view)->fallen_list = g_list_remove( PRIVATE(gdk_view)->fallen_list
                                                                        , block);
                        gdk_canvas_remove_block( PRIVATE(gdk_view)->canvas,
                                                 block);
                        gdk_canvas_unref_block( PRIVATE(gdk_view)->canvas,
                                                block);
 
                } else {
                        gdk_canvas_move_block( PRIVATE(gdk_view)->canvas,
                                               block,
                                               x,y+10);
      
                }
	
                next = g_list_next( next);
	
        }


        next = PRIVATE(gdk_view)->added_list;
    
        while( next != NULL ) {
	
                added = (BubbleAdded *) next->data;
                block_get_position( added->block,&x,&y);
                bubble_get_position( added->bubble,&x2,&y2);
                if( (y -30) < y2 ) {
	    
                        next = g_list_previous( next );
                        gdk_canvas_move_block(PRIVATE(gdk_view)->canvas,
                                              added->block,x2,y2 );
	    
                        PRIVATE(gdk_view)->added_list = g_list_remove(PRIVATE(gdk_view)->added_list,
                                                                      added);
                        g_hash_table_insert( PRIVATE(gdk_view)->hash_map, 
                                             added->bubble, 
                                             added->block );



                        g_signal_connect( G_OBJECT(added->bubble), "bubble-changed", G_CALLBACK( gdk_view_bubble_changed), gdk_view);
                        g_free(added);
                } else {
                        gdk_canvas_move_block( PRIVATE(gdk_view)->canvas,
                                               added->block,x,y-50);
                }
	
                next = g_list_next( next);
        }

        gdk_view_animate_stars(gdk_view,dtime);
}


static void gdk_view_animate_stars(GdkView * d,gint time) {
        GList * next;
        Star * star;
        gdouble x,y;
        next = PRIVATE(d)->star_list;
        
        while(next != NULL) {
                //  g_print("time %d",time);
                star = (Star *)next->data;
                block_get_position( star->block, &x,&y);
                x +=  star->vx;
                y +=  star->vy;

                star->vx = star->vx;
                
                star->vy = star->vy+0.2;
                gdk_canvas_move_block( PRIVATE(d)->canvas, star->block, x,y);
                next = g_list_next( next);
        }
}

static void gdk_view_instance_init(GdkView * gdk_view) {
        gdk_view->private =g_new0 (GdkViewPrivate, 1);			
}

static void gdk_view_finalize(GObject* object) {
        Playground * p;
        GdkView * gdk_view = GDK_VIEW(object);

        p = monkey_get_playground( PRIVATE(gdk_view)->monkey);



        g_signal_handlers_disconnect_matched( G_OBJECT( PRIVATE(gdk_view)->monkey),
                                              G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,gdk_view);

        g_signal_handlers_disconnect_matched(  G_OBJECT( playground_get_board(p) ),
                                               G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,gdk_view);

        g_signal_handlers_disconnect_matched( G_OBJECT(monkey_get_shooter(PRIVATE(gdk_view)->monkey)),
                                              G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,gdk_view);



        g_hash_table_foreach(PRIVATE(gdk_view)->hash_map,
                             gdk_view_free_map,
                             gdk_view);
  


        g_object_unref( PRIVATE(gdk_view)->monkey);
  

        //  gdk_canvas_clear( PRIVATE(gdk_view)->canvas);
        //  g_object_unref( PRIVATE(gdk_view)-> canvas);
        g_free(gdk_view->private);
  
        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static void gdk_view_free_map(gpointer key,
			      gpointer value,
			      gpointer user_data) {
        GdkView * view;
        Bubble *b;
        Block * bb;

        view = GDK_VIEW(user_data);
        b = BUBBLE(key);
        bb = (Block *) value;



        g_signal_handlers_disconnect_matched( G_OBJECT(b),G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,view);

}

static void gdk_view_class_init (GdkViewClass *klass) {
        GObjectClass* object_class;
    
        parent_class = g_type_class_peek_parent(klass);
        object_class = G_OBJECT_CLASS(klass);
        object_class->finalize = gdk_view_finalize;
}


GType gdk_view_get_type(void) {
        static GType gdk_view_type = 0;
    
        if (!gdk_view_type) {
                static const GTypeInfo gdk_view_info = {
                        sizeof(GdkViewClass),
                        NULL,           /* base_init */
                        NULL,           /* base_finalize */
                        (GClassInitFunc) gdk_view_class_init,
                        NULL,           /* class_finalize */
                        NULL,           /* class_data */
                        sizeof(GdkView),
                        1,              /* n_preallocs */
                        (GInstanceInitFunc) gdk_view_instance_init,
                };


                gdk_view_type = g_type_register_static(G_TYPE_OBJECT,
                                                       "GdkView",
                                                       &gdk_view_info, 0);





        }
    
        return gdk_view_type;
}






static void gdk_view_bubble_changed(Bubble * b,GdkView * view) {
        gdouble x,y;
  
        g_assert( IS_GDK_VIEW( view ) );

        bubble_get_position( b , &x , &y);

        gdk_canvas_move_block( PRIVATE(view)->canvas,
                               (Block *)g_hash_table_lookup( PRIVATE(view)->hash_map,
                                                             b ),
                               x,y
                               );
}






static void gdk_view_shooter_rotated(Shooter * shooter,
				     GdkView * p) {


        Block * b;
        gdouble t;
        gdouble x,y;

        t = shooter_get_angle( shooter )/ M_PI * 80;
        if( t < 0 ) {
                b = PRIVATE(p)->shooter_block[ 41 - (int) t];
        } else {
                b = PRIVATE(p)->shooter_block[ (int) t];
        }

        shooter_get_position(shooter,&x,&y);

        gdk_canvas_remove_block(PRIVATE(p)->canvas,
                                PRIVATE(p)->current_shooter_block);
        gdk_canvas_add_block( PRIVATE(p)->canvas,
                              PRIVATE(p)->shooter_layer,
                              b,
                              x,
                              y );

        PRIVATE(p)->current_shooter_block = b;
}

static void gdk_view_shooter_bubble_added(Shooter * s,
					  Bubble * b,
					  GdkView * d) {
        gdouble x,y;
        Block * block;
        g_assert(IS_GDK_VIEW(d) );


        bubble_get_position(b,&x,&y);
        block = gdk_view_create_bubble(d,b);
        gdk_canvas_add_block(PRIVATE(d)->canvas,
                             PRIVATE(d)->bubble_layer , 
                             block,
                             x,y);

        g_hash_table_insert( PRIVATE(d)->hash_map, b, block );

        g_signal_connect(G_OBJECT(b),"bubble-changed",G_CALLBACK( gdk_view_bubble_changed), d);
}

static Block * gdk_view_create_bubble(GdkView * view,
				      Bubble * bubble ) {


        gchar path[4096];
        gint str_length;
        Block * block;


        strcpy( path ,
                DATADIR"/monkey-bubble/gfx/bubbles/bubble_");
	
        str_length = strlen(path);

    
        sprintf(path+str_length ,"0%d.svg",bubble_get_color(bubble)+1);
    
    
        block = gdk_canvas_create_block_from_image(PRIVATE(view)->canvas,
                                                   path,
                                                   33,33,
                                                   16,16);

        return block;
}


static void gdk_view_board_down(Board * board,GdkView * gdk_view) {

  
        g_assert( IS_GDK_VIEW( gdk_view) );
  
        gdk_canvas_move_block( PRIVATE(gdk_view)->canvas,
                               PRIVATE(gdk_view)->bback,
                               320,
                               board_get_y_min( board )-10);
}

static void gdk_view_bubbles_exploded(   Board * board,
                                         GList * exploded,
                                         GList * fallen,
                                         GdkView * gdk_view) {

        GList * next;
        Bubble * bubble;
        Block * block;


        next = exploded;

        while( next != NULL ) {
                bubble = next->data;
                gdk_view_add_explode_stars(gdk_view,bubble);

                block = (Block *)g_hash_table_lookup( PRIVATE(gdk_view)->hash_map,
                                                      bubble );			    
                gdk_canvas_remove_block( PRIVATE(gdk_view)->canvas,
                                         block);

                gdk_canvas_unref_block( PRIVATE(gdk_view)->canvas,
                                        block);

                g_hash_table_remove(PRIVATE(gdk_view)->hash_map,
                                    bubble);

                g_signal_handlers_disconnect_matched( G_OBJECT(bubble),G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,gdk_view);
                next = g_list_next(next);
        }


        next = fallen;
        while( next != NULL ) {
                bubble = next->data;
                PRIVATE(gdk_view)->fallen_list = 
                        g_list_append(     PRIVATE(gdk_view)->fallen_list,
                                           (Block *)g_hash_table_lookup( PRIVATE(gdk_view)->hash_map,
                                                                         bubble )

                                           );


                g_hash_table_remove(PRIVATE(gdk_view)->hash_map,
                                    bubble);


                g_signal_handlers_disconnect_matched( G_OBJECT(bubble),G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,gdk_view);
                next = g_list_next(next);
        }
}

void gdk_view_draw_lost(GdkView *d) {
        g_assert(IS_GDK_VIEW(d));

        gdk_canvas_add_block( PRIVATE(d)->canvas,
                              PRIVATE(d)->monkeys_layer,
                              PRIVATE(d)->lost,
                              320,240);
}

static void gdk_view_add_explode_stars(GdkView * d,Bubble * b) {
        Star * star;
        int i;
        gdouble dx,dy;

        for(i= 0; i < 2; i++) {
                star = g_malloc( sizeof(Star));
                bubble_get_position( b, &dx,&dy);

                
                star->vx = (rand()%400)/100 -2;
                star->vy = -(rand()%200)/ 100;
                //0.1;

                star->block = gdk_canvas_create_block_from_image( PRIVATE(d)->canvas,
                                                                  DATADIR"/monkey-bubble/gfx/star.svg",
                                                                  16,16,
                                                                  8,8);

                gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->star_layer,
                                      star->block,
                                      dx,dy);

                PRIVATE(d)->star_list = g_list_append( PRIVATE(d)->star_list,
                                                       star);
        }
        
}

static Block * gdk_view_create_gem(GdkView * d) {

        Block * block;  
    
        block = gdk_canvas_create_block_from_image(PRIVATE(d)->canvas,
                                                   DATADIR"/monkey-bubble/gfx/banana.svg",
                                                   32,32,
                                                   8,8);
  
        return block;
  
}

static void gdk_view_clear_gems(GdkView * d) {
        Block * block;
        GList * next;

        next = PRIVATE(d)->gems_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                gdk_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                gdk_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->gems_list);
        PRIVATE(d)->gems_list = NULL;

}

void gdk_view_set_gems_count(GdkView * d,int gems) {
        Block * block;
        g_assert(IS_GDK_VIEW(d));
  
        gdk_view_clear_gems(d);


        while(gems > 0 ) {
		block = gdk_view_create_gem(d);
		gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->shooter_layer,
                                      block,
                                      40,200+(gems*30));
  
		PRIVATE(d)->gems_list = g_list_append( PRIVATE(d)->gems_list,block);
		gems--;
        }

}


static Block * gdk_view_create_number(GdkView * view,int number) {
        gchar path[4096];
        gint str_length;
        Block * block;


        strcpy( path ,
                DATADIR"/monkey-bubble/gfx/number/");
	
        str_length = strlen(path);

    
        sprintf(path+str_length ,"%d.svg",number);

        block = gdk_canvas_create_block_from_image(PRIVATE(view)->canvas,
                                                   path,
                                                   32,32,
                                                   16,16);
  
        return block;

}

static void gdk_view_clear_score(GdkView * d) {

        Block * block;
        GList * next;

        next = PRIVATE(d)->score_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                gdk_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                gdk_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->score_list);
        PRIVATE(d)->score_list = NULL;
}


void gdk_view_set_score(GdkView * d,int score) {

        Block * block;
        int pre_score;
        int i = 0;
  
    
        g_assert(IS_GDK_VIEW(d));
        gdk_view_clear_score(d);



        if( score != 0 ) {

                while( score != 0 ) {
                        pre_score = score % 10 ;

                        block = gdk_view_create_number(d,pre_score);  
                        gdk_canvas_add_block( PRIVATE(d)->canvas,
                                              PRIVATE(d)->shooter_layer,
                                              block,
                                              450-i*20,415);

                        PRIVATE(d)->score_list = g_list_append( PRIVATE(d)->score_list,block);

                        score = score / 10;
                        i++;
                }
        } else {
                block = gdk_view_create_number(d,0);  
                gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->shooter_layer,
                                      block,
                                      450,415);

                PRIVATE(d)->score_list = g_list_append( PRIVATE(d)->score_list,block);

        }

}


static void gdk_view_clear_points(GdkView * d) {

        Block * block;
        GList * next;

        next = PRIVATE(d)->points_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                gdk_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                gdk_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->points_list);
        PRIVATE(d)->points_list = NULL;
}


void gdk_view_set_points(GdkView * d,int score) {

        Block * block;
        int pre_score;
        int i = 0;
  
    
        g_assert(IS_GDK_VIEW(d));
        gdk_view_clear_points(d);



        if( score != 0 ) {

                while( score != 0 ) {
                        pre_score = score % 10 ;

                        block = gdk_view_create_number(d,pre_score);  
                        gdk_canvas_add_block( PRIVATE(d)->canvas,
                                              PRIVATE(d)->shooter_layer,
                                              block,
                                              600-i*20,450);

                        PRIVATE(d)->points_list = g_list_append( PRIVATE(d)->points_list,block);

                        score = score / 10;
                        i++;
                }
        } else {
                block = gdk_view_create_number(d,0);  
                gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->shooter_layer,
                                      block,
                                      600,450);

                PRIVATE(d)->points_list = g_list_append( PRIVATE(d)->points_list,block);

        }

}



static Block * gdk_view_create_little_waiting(GdkView * d) {

        Block * block;  
    
        block = gdk_canvas_create_block_from_image(PRIVATE(d)->canvas,
                                                   DATADIR"/monkey-bubble/gfx/banana.svg",
                                                   32,32,
                                                   16,16);
  
        return block;
  
}


static Block * gdk_view_create_big_waiting(GdkView * d) {

        Block * block;  
    
        block = gdk_canvas_create_block_from_image(PRIVATE(d)->canvas,
                                                   DATADIR"/monkey-bubble/gfx/tomato.svg",
                                                   32,32,
                                                   16,16);
  
        return block;
  
}

static void gdk_view_clear_waiting(GdkView * d) {
        Block * block;
        GList * next;

        next = PRIVATE(d)->waiting_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                gdk_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                gdk_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->waiting_list);
        PRIVATE(d)->waiting_list = NULL;
}

void gdk_view_set_waiting_bubbles(GdkView * d,int bubbles) {
        Block * block;
        int i;
        g_assert(IS_GDK_VIEW(d));
  
        gdk_view_clear_waiting(d);
  
        i = 0;
        while(bubbles > 0 ) {
		if( bubbles >= 5 ) {

                        bubbles = bubbles - 5;

                        block = gdk_view_create_big_waiting(d);
		} else {
                        bubbles--;

                        block = gdk_view_create_little_waiting(d);
		}


		gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->bubble_layer,
                                      block,
                                      185,400-(i*25));

		
		PRIVATE(d)->waiting_list = g_list_append( PRIVATE(d)->waiting_list,block);
		i++;
        }


}


void gdk_view_draw_win(GdkView *d) {
        g_assert(IS_GDK_VIEW(d));
        gdk_canvas_add_block( PRIVATE(d)->canvas,
                              PRIVATE(d)->monkeys_layer,
                              PRIVATE(d)->win,
                              320,240);
}


static void gdk_view_bubbles_inserted(
				      Board * board,
				      Bubble ** bubbles,
				      int count,
				      GdkView *d) {

        int i;
        gdouble x,y;
        Block * block;
        Bubble * b;

        for(i = 0; i < count; i++) {
                b = bubbles[i];
                bubble_get_position(b,&x,&y);
      
                block = gdk_view_create_bubble(d,
                                               b);
      
                g_hash_table_insert( PRIVATE(d)->hash_map, 
                                     b, 
                                     block );
                gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->bubble_layer,
                                      block,
                                      x,y);

                g_signal_connect( G_OBJECT(b),"bubble-changed",G_CALLBACK(gdk_view_bubble_changed),d);
      
        }
}

static void gdk_view_bubbles_added(Board * board,
				   GList * bubbles,
				   GdkView *d) {
    
        gdouble x,y;
        Block * block;
        GList * next;
        BubbleAdded * added;
        Bubble * b;

        next = bubbles;

        while( next != NULL ) {
                b = (Bubble *) next->data;

                bubble_get_position(b,&x,&y);
                block = gdk_view_create_bubble(d,
                                               b);
    
                gdk_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->bubble_layer,
                                      block,
                                      x,480);
        
	
                added = g_malloc( sizeof(BubbleAdded) );
                added->bubble = b;
                added->block = block;
                PRIVATE(d)->added_list = g_list_append( PRIVATE(d)->added_list,
                                                        added);
                next = g_list_next( next );
        }
}





static void gdk_view_bubbles_waiting_changed(Monkey * monkey,
					     int bubbles_count,
					     GdkView * view) {

        gdk_view_set_waiting_bubbles(view,bubbles_count);
}
