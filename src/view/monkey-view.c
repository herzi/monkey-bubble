/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* monkey_view.c
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
#include "monkey-view.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUBBLE_COUNT 8
#define SHOOTER_COUNT 40*2

#define PRIVATE( monkey_view ) (monkey_view->private)
static GObjectClass* parent_class = NULL;

typedef struct Star {
        gdouble vx,vy;
        Block * block;
} Star;

typedef struct BubbleAdded {
        Block * block;
        Bubble * bubble;
    
} BubbleAdded;

struct MonkeyViewPrivate {
        gint last_time;
        MonkeyCanvas * canvas;
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
        Block * harm_up;
        Block * harm_down;
        Block * harm_center;
        Block * harm_shoot;
        Block * monkeys;
        GList * waiting_list;
        GList * score_list;
        GList * points_list;
        GList * gems_list;
        GList * star_list;
        gint time;
        gint last_shoot;
};

static void monkey_view_animate_stars(MonkeyView * d,gint time);
static void monkey_view_add_explode_stars(MonkeyView * d,Bubble * b) ;
static void monkey_view_bubble_changed(Bubble * b,MonkeyView * view);

static void monkey_view_bubbles_waiting_changed(Monkey * monkey,
					     int bubbles_count,
					     MonkeyView * view);

static Block * monkey_view_create_gem(MonkeyView * d);
static Block * monkey_view_create_big_waiting(MonkeyView * d);
static Block * monkey_view_create_little_waiting(MonkeyView * d);
static void monkey_view_free_map(gpointer key,
			      gpointer value,
			      gpointer user_data);

static void monkey_view_shooter_bubble_added(Shooter * shooter,
					  Bubble * b,
					  MonkeyView * view);

static void monkey_view_shooter_rotated(Shooter * shooter,MonkeyView * view);

static void monkey_view_bubbles_exploded(    Board * board,
                                          GList * exploed,
                                          GList * fallen,
					  MonkeyView * view);
static void monkey_view_bubbles_added(  Board * board,
                                     GList * bubbles,
				     MonkeyView * view);


static void monkey_view_board_down(Board * board,
				MonkeyView * view);


static void monkey_view_bubbles_inserted(    Board * board,
                                          Bubble ** bubbles,
                                          int count,
					  MonkeyView * view);




static Block * monkey_view_create_bubble(MonkeyView * view,
				      Bubble * bubble );
void monkey_view_load_shooter_images(MonkeyView * monkey_view) {
        GError * error;
        gchar path[4096];
        gint str_length;
        gint i;
        Shooter * shooter;
  
        gdouble x,y;

  
        error = NULL;
        shooter = monkey_get_shooter(PRIVATE(monkey_view)->monkey);

  
  
        strcpy( path,DATADIR"/monkey-bubble/gfx/snake/snake_");
  
        str_length  = strlen(path);
        shooter_get_position( shooter,&x,&y);
  
  
        for( i=0; i < SHOOTER_COUNT/2+1 ; ++i) {

      
		snprintf(path+str_length ,7,"%d.svg",i);    
                
    
                PRIVATE(monkey_view)->shooter_block[i] = 
                        monkey_canvas_create_block_from_image(PRIVATE(monkey_view)->canvas,
                                                           path,120,60,
                                                           60 ,					   
                                                           40 );

        }
  

  
        for( i=SHOOTER_COUNT/2+1; i < SHOOTER_COUNT+1 ; i++) {
      
                snprintf(path+str_length ,8,"-%d.svg",i-SHOOTER_COUNT/2);
      
      
      
                PRIVATE(monkey_view)->shooter_block[i] = 
                        monkey_canvas_create_block_from_image(PRIVATE(monkey_view)->canvas,
                                                           path,120,60,
                                                           60,					   
                                                           40);
        }
      
}

void monkey_view_set_harm(MonkeyView * monkey_view,Block * h) {
        if( PRIVATE(monkey_view)->harm != h  ) {
        if( PRIVATE(monkey_view)->harm != NULL) {
                monkey_canvas_remove_block( PRIVATE(monkey_view)->canvas,
                                            PRIVATE(monkey_view)->harm);
        }

        PRIVATE(monkey_view)->harm = h;
        monkey_canvas_add_block( PRIVATE(monkey_view)->canvas,
                                 PRIVATE(monkey_view)->ground_layer,
                                 PRIVATE(monkey_view)->harm,
                                 310,386);
        }
}


static void monkey_view_shooter_down(Monkey * m,MonkeyView * p) {

        monkey_view_set_harm( p,PRIVATE(p)->harm_down);
}



static void monkey_view_shooter_up(Monkey * m,MonkeyView * p) {

        monkey_view_set_harm( p,PRIVATE(p)->harm_up);
}



static void monkey_view_shooter_center(Monkey * m,MonkeyView * p) {
        if( PRIVATE(p)->harm != PRIVATE(p)->harm_shoot || 
            PRIVATE(p)->last_shoot + 200 < PRIVATE(p)->time) {
                monkey_view_set_harm( p,PRIVATE(p)->harm_center);
        }
}



static void monkey_view_shooter_shoot(Shooter * s,Bubble *b ,MonkeyView * p) {

        PRIVATE(p)->last_shoot = PRIVATE(p)->time;
        monkey_view_set_harm( p,PRIVATE(p)->harm_shoot);
}


MonkeyView * monkey_view_new(MonkeyCanvas * canvas,
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

        MonkeyView * monkey_view = MONKEY_VIEW (g_object_new (TYPE_MONKEY_VIEW, NULL));

        error = NULL;
  
        g_assert( IS_MONKEY_VIEW( monkey_view ) );

        g_object_ref( monkey);
        PRIVATE(monkey_view)->monkey = monkey;


        PRIVATE(monkey_view)->last_time = 0;
        PRIVATE(monkey_view)->canvas = canvas;

        PRIVATE(monkey_view)->added_list = NULL;
  
        PRIVATE(monkey_view)->background_layer = monkey_canvas_get_root_layer( canvas );
        PRIVATE(monkey_view)->ground_layer = monkey_canvas_append_layer(canvas,x_pos,y_pos);
        PRIVATE(monkey_view)->shooter_layer = monkey_canvas_append_layer( canvas ,x_pos,y_pos);
        PRIVATE(monkey_view)->bubble_layer = monkey_canvas_append_layer( canvas,x_pos,y_pos );
        PRIVATE(monkey_view)->monkeys_layer = monkey_canvas_append_layer(canvas,x_pos,y_pos);
        PRIVATE(monkey_view)->star_layer = monkey_canvas_append_layer(canvas,x_pos,y_pos);
  
  
        PRIVATE(monkey_view)->hash_map = 
                g_hash_table_new(g_direct_hash,g_direct_equal);
  
  
  
        monkey_view_load_shooter_images(monkey_view);
  
        if(back_needed ) {
      

                PRIVATE(monkey_view)->background = 
                        monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                            DATADIR"/monkey-bubble/gfx/layout_1_player.svg",
                                                            640,480,0,0);
      
                monkey_canvas_add_block( PRIVATE(monkey_view)->canvas,
                                      PRIVATE(monkey_view)->background_layer,
                                      PRIVATE(monkey_view)->background,
                                      0,0);

	
        } else {
		  
                PRIVATE(monkey_view)->background =   NULL;
        }
	 
        PRIVATE(monkey_view)->bback =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                        DATADIR"/monkey-bubble/gfx/pane.svg",
                                                                        280,80,140,40);
        monkey_canvas_add_block( PRIVATE(monkey_view)->canvas,
                              PRIVATE(monkey_view)->ground_layer,
                              PRIVATE(monkey_view)->bback,
                              320,30);
	 

        PRIVATE(monkey_view)->monkeys =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                          DATADIR"/monkey-bubble/gfx/monkeys.svg",
                                                                          300,160,150,0);

        PRIVATE(monkey_view)->harm = NULL;
        PRIVATE(monkey_view)->harm_up =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                       DATADIR"/monkey-bubble/gfx/harm_up.svg",
                                                                       90,60,0,0);

        PRIVATE(monkey_view)->harm_down =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                       DATADIR"/monkey-bubble/gfx/harm_down.svg",
                                                                       90,60,0,0);

        PRIVATE(monkey_view)->harm_center =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                       DATADIR"/monkey-bubble/gfx/harm.svg",
                                                                       90,60,0,0);

        PRIVATE(monkey_view)->harm_shoot =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                       DATADIR"/monkey-bubble/gfx/harm_shoot.svg",
                                                                       90,60,0,0);



        

        monkey_view_set_harm( monkey_view, PRIVATE(monkey_view)->harm_center);

        monkey_canvas_add_block( PRIVATE(monkey_view)->canvas,
                              PRIVATE(monkey_view)->monkeys_layer,
                              PRIVATE(monkey_view)->monkeys,
                              320,350);





        PRIVATE(monkey_view)->snake_body =  monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                                             DATADIR"/monkey-bubble/gfx/snake-body.svg",
                                                                             60,60,30,20);	 
        PRIVATE(monkey_view)->lost = 
                monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                    DATADIR"/monkey-bubble/gfx/lost.svg",
                                                    200,200,
                                                    100,100);
	 
        PRIVATE(monkey_view)->win = 
                monkey_canvas_create_block_from_image( PRIVATE(monkey_view)->canvas,
                                                    DATADIR"/monkey-bubble/gfx/win.svg",
                                                    200,200,
                                                    100,100);
	 
    
        shooter = monkey_get_shooter(PRIVATE(monkey_view)->monkey);
        shooter_get_position(shooter,&x,&y);
    
        PRIVATE(monkey_view)->current_shooter_block = PRIVATE(monkey_view)->shooter_block[0];
    
        monkey_canvas_add_block( PRIVATE(monkey_view)->canvas,
                              PRIVATE(monkey_view)->shooter_layer,
                              PRIVATE(monkey_view)->shooter_block[0],
                              x,y);

        monkey_canvas_add_block( PRIVATE(monkey_view)->canvas,
                              PRIVATE(monkey_view)->monkeys_layer,
                              PRIVATE(monkey_view)->snake_body,
                              x,y);
    
        if( shooter_get_current_bubble( shooter ) != NULL ) {
                monkey_view_shooter_bubble_added(
                                              shooter,
                                              shooter_get_current_bubble( shooter ), monkey_view);
	
        }
    
    
        if( shooter_get_waiting_bubble( shooter ) != NULL ) {
                monkey_view_shooter_bubble_added(
                                              shooter,
                                              shooter_get_waiting_bubble( shooter ),monkey_view);
		  
        }

	PRIVATE(monkey_view)->star_list = NULL;
        PRIVATE(monkey_view)->score_list = NULL;
        PRIVATE(monkey_view)->gems_list = NULL;
        PRIVATE(monkey_view)->waiting_list = NULL;

        PRIVATE(monkey_view)->fallen_list = NULL;
        PRIVATE(monkey_view)->time = 0;
  
        pl = monkey_get_playground( monkey);

        g_signal_connect( G_OBJECT(monkey),"bubbles-waiting-changed",
                          G_CALLBACK( monkey_view_bubbles_waiting_changed),
                          monkey_view);

  
        g_signal_connect( G_OBJECT(shooter),
                          "rotated",
                          G_CALLBACK( monkey_view_shooter_rotated), 
                          monkey_view);		    

        g_signal_connect( G_OBJECT(shooter),
                          "shoot",
                          G_CALLBACK( monkey_view_shooter_shoot), 
                          monkey_view);		    

  
        g_signal_connect( G_OBJECT(monkey),
                          "up",
                          G_CALLBACK( monkey_view_shooter_up), 
                          monkey_view);		    

        g_signal_connect( G_OBJECT(monkey),
                          "down",
                          G_CALLBACK( monkey_view_shooter_down), 
                          monkey_view);		    

        g_signal_connect( G_OBJECT(monkey),
                          "center",
                          G_CALLBACK( monkey_view_shooter_center), 
                          monkey_view);		    

        g_signal_connect( G_OBJECT(shooter),
                          "bubble-added",
                          G_CALLBACK( monkey_view_shooter_bubble_added), 
                          monkey_view);		    

        board = playground_get_board( monkey_get_playground( monkey));
 
        g_signal_connect( G_OBJECT( board),
                          "bubbles-exploded",G_CALLBACK(monkey_view_bubbles_exploded),monkey_view);

 
        g_signal_connect( G_OBJECT( board),
                          "bubbles-added",G_CALLBACK(monkey_view_bubbles_added),monkey_view);

 
        g_signal_connect( G_OBJECT( board),
                          "bubbles-inserted",G_CALLBACK(monkey_view_bubbles_inserted),monkey_view);

 
        g_signal_connect( G_OBJECT( board),
                          "down",G_CALLBACK(monkey_view_board_down),monkey_view);


  
        for( j = 0; j < board_get_row_count( board); j++) {
      
                for( i = 0; i < board_get_column_count( board); i++) {
	  
                        if( ( b = board_get_bubble_at(board,i,j) )!= NULL ) {
                                bubble_get_position(b,&x,&y);
                                block = monkey_view_create_bubble(monkey_view,b);
                                monkey_canvas_add_block(PRIVATE(monkey_view)->canvas,
                                                     PRIVATE(monkey_view)->bubble_layer , 
                                                     block,
                                                     x,y);
	
                                g_hash_table_insert( PRIVATE(monkey_view)->hash_map, b, block );
	
                                g_signal_connect( G_OBJECT(b), "bubble-changed", G_CALLBACK( monkey_view_bubble_changed), monkey_view);
	
	
                        }
      
                }
    
        }
  
        return monkey_view;
}


void monkey_view_update(MonkeyView * monkey_view,
		     gint time) {
    
        GList * next;
        Block * block;
        gdouble x,y,x2,y2;
        BubbleAdded * added;
        gint dtime;
        g_assert( IS_MONKEY_VIEW(monkey_view));
    
        next = PRIVATE(monkey_view)->fallen_list;
        
        dtime = time - PRIVATE(monkey_view)->time;
        PRIVATE(monkey_view)->time = time;
        
        while( next != NULL ) {
	
                block = next->data;
                block_get_position( block ,&x,&y);

                if( y > 480) {
                        next = g_list_previous(next);
                        PRIVATE(monkey_view)->fallen_list = g_list_remove( PRIVATE(monkey_view)->fallen_list
                                                                        , block);
                        monkey_canvas_remove_block( PRIVATE(monkey_view)->canvas,
                                                 block);
                        monkey_canvas_unref_block( PRIVATE(monkey_view)->canvas,
                                                block);
 
                } else {
                        monkey_canvas_move_block( PRIVATE(monkey_view)->canvas,
                                                  block,
                                                  x,y+ dtime/2.0);
      
                }
	
                next = g_list_next( next);
	
        }


        next = PRIVATE(monkey_view)->added_list;
    
        while( next != NULL ) {
	
                added = (BubbleAdded *) next->data;
                block_get_position( added->block,&x,&y);
                bubble_get_position( added->bubble,&x2,&y2);
                if( (y -30) < y2 ) {
	    
                        next = g_list_previous( next );
                        monkey_canvas_move_block(PRIVATE(monkey_view)->canvas,
                                              added->block,x2,y2 );
	    
                        PRIVATE(monkey_view)->added_list = g_list_remove(PRIVATE(monkey_view)->added_list,
                                                                      added);
                        g_hash_table_insert( PRIVATE(monkey_view)->hash_map, 
                                             added->bubble, 
                                             added->block );



                        g_signal_connect( G_OBJECT(added->bubble), "bubble-changed", G_CALLBACK( monkey_view_bubble_changed), monkey_view);
                        g_free(added);
                } else {
                        monkey_canvas_move_block( PRIVATE(monkey_view)->canvas,
                                               added->block,x,y-50);
                }
	
                next = g_list_next( next);
        }

        monkey_view_animate_stars(monkey_view,dtime);
}


static void monkey_view_animate_stars(MonkeyView * d,gint time) {
        GList * next;
        Star * star;
        gdouble x,y;
        next = PRIVATE(d)->star_list;
        
        while(next != NULL) {
                //  g_print("time %d",time);
                star = (Star *)next->data;
                block_get_position( star->block, &x,&y);
                x +=  (star->vx * time)/100 ;
                y +=  (star->vy * time)/100;
                
                /*                if( star -> vx > 0 ) {
                        star->vx = star->vx - time/200.0;
                }

                if( star -> vx < 0 ) {
                        star->vx = star->vx + time/200.0;
                }
                */
                star->vy = star->vy + time/10.0;
                monkey_canvas_move_block( PRIVATE(d)->canvas, star->block, x,y);
                next = g_list_next( next);
        }
}

static void monkey_view_instance_init(MonkeyView * monkey_view) {
        monkey_view->private =g_new0 (MonkeyViewPrivate, 1);			
}

static void monkey_view_finalize(GObject* object) {
        Playground * p;
        MonkeyView * monkey_view = MONKEY_VIEW(object);

        p = monkey_get_playground( PRIVATE(monkey_view)->monkey);



        g_signal_handlers_disconnect_matched( G_OBJECT( PRIVATE(monkey_view)->monkey),
                                              G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey_view);

        g_signal_handlers_disconnect_matched(  G_OBJECT( playground_get_board(p) ),
                                               G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey_view);

        g_signal_handlers_disconnect_matched( G_OBJECT(monkey_get_shooter(PRIVATE(monkey_view)->monkey)),
                                              G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey_view);



        g_hash_table_foreach(PRIVATE(monkey_view)->hash_map,
                             monkey_view_free_map,
                             monkey_view);
  


        g_object_unref( PRIVATE(monkey_view)->monkey);
  

        //  monkey_canvas_clear( PRIVATE(monkey_view)->canvas);
        //  g_object_unref( PRIVATE(monkey_view)-> canvas);
        g_free(monkey_view->private);
  
        if (G_OBJECT_CLASS (parent_class)->finalize) {
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
        }
}

static void monkey_view_free_map(gpointer key,
			      gpointer value,
			      gpointer user_data) {
        MonkeyView * view;
        Bubble *b;
        Block * bb;

        view = MONKEY_VIEW(user_data);
        b = BUBBLE(key);
        bb = (Block *) value;



        g_signal_handlers_disconnect_matched( G_OBJECT(b),G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,view);

}

static void monkey_view_class_init (MonkeyViewClass *klass) {
        GObjectClass* object_class;
    
        parent_class = g_type_class_peek_parent(klass);
        object_class = G_OBJECT_CLASS(klass);
        object_class->finalize = monkey_view_finalize;
}


GType monkey_view_get_type(void) {
        static GType monkey_view_type = 0;
    
        if (!monkey_view_type) {
                static const GTypeInfo monkey_view_info = {
                        sizeof(MonkeyViewClass),
                        NULL,           /* base_init */
                        NULL,           /* base_finalize */
                        (GClassInitFunc) monkey_view_class_init,
                        NULL,           /* class_finalize */
                        NULL,           /* class_data */
                        sizeof(MonkeyView),
                        1,              /* n_preallocs */
                        (GInstanceInitFunc) monkey_view_instance_init,
                };


                monkey_view_type = g_type_register_static(G_TYPE_OBJECT,
                                                       "MonkeyView",
                                                       &monkey_view_info, 0);





        }
    
        return monkey_view_type;
}






static void monkey_view_bubble_changed(Bubble * b,MonkeyView * view) {
        gdouble x,y;
  
        g_assert( IS_MONKEY_VIEW( view ) );

        bubble_get_position( b , &x , &y);

        monkey_canvas_move_block( PRIVATE(view)->canvas,
                               (Block *)g_hash_table_lookup( PRIVATE(view)->hash_map,
                                                             b ),
                               x,y
                               );
}






static void monkey_view_shooter_rotated(Shooter * shooter,
				     MonkeyView * p) {


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

        monkey_canvas_remove_block(PRIVATE(p)->canvas,
                                PRIVATE(p)->current_shooter_block);
        monkey_canvas_add_block( PRIVATE(p)->canvas,
                              PRIVATE(p)->shooter_layer,
                              b,
                              x,
                              y );

        PRIVATE(p)->current_shooter_block = b;
}

static void monkey_view_shooter_bubble_added(Shooter * s,
					  Bubble * b,
					  MonkeyView * d) {
        gdouble x,y;
        Block * block;
        g_assert(IS_MONKEY_VIEW(d) );


        bubble_get_position(b,&x,&y);
        block = monkey_view_create_bubble(d,b);
        monkey_canvas_add_block(PRIVATE(d)->canvas,
                             PRIVATE(d)->bubble_layer , 
                             block,
                             x,y);

        g_hash_table_insert( PRIVATE(d)->hash_map, b, block );

        g_signal_connect(G_OBJECT(b),"bubble-changed",G_CALLBACK( monkey_view_bubble_changed), d);
}

static Block * monkey_view_create_bubble(MonkeyView * view,
				      Bubble * bubble ) {


        gchar path[4096];
        gint str_length;
        Block * block;


        strcpy( path ,
                DATADIR"/monkey-bubble/gfx/bubbles/bubble_");
	
        str_length = strlen(path);

    
        sprintf(path+str_length ,"0%d.svg",bubble_get_color(bubble)+1);
    
    
        block = monkey_canvas_create_block_from_image(PRIVATE(view)->canvas,
                                                   path,
                                                   33,33,
                                                   16,16);

        return block;
}


static void monkey_view_board_down(Board * board,MonkeyView * monkey_view) {

  
        g_assert( IS_MONKEY_VIEW( monkey_view) );
  
        monkey_canvas_move_block( PRIVATE(monkey_view)->canvas,
                               PRIVATE(monkey_view)->bback,
                               320,
                               board_get_y_min( board )-10);
}

static void monkey_view_bubbles_exploded(   Board * board,
                                         GList * exploded,
                                         GList * fallen,
                                         MonkeyView * monkey_view) {

        GList * next;
        Bubble * bubble;
        Block * block;


        next = exploded;

        while( next != NULL ) {
                bubble = next->data;
                monkey_view_add_explode_stars(monkey_view,bubble);

                block = (Block *)g_hash_table_lookup( PRIVATE(monkey_view)->hash_map,
                                                      bubble );			    
                monkey_canvas_remove_block( PRIVATE(monkey_view)->canvas,
                                         block);

                monkey_canvas_unref_block( PRIVATE(monkey_view)->canvas,
                                        block);

                g_hash_table_remove(PRIVATE(monkey_view)->hash_map,
                                    bubble);

                g_signal_handlers_disconnect_matched( G_OBJECT(bubble),G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey_view);
                next = g_list_next(next);
        }


        next = fallen;
        while( next != NULL ) {
                bubble = next->data;
                PRIVATE(monkey_view)->fallen_list = 
                        g_list_append(     PRIVATE(monkey_view)->fallen_list,
                                           (Block *)g_hash_table_lookup( PRIVATE(monkey_view)->hash_map,
                                                                         bubble )

                                           );


                g_hash_table_remove(PRIVATE(monkey_view)->hash_map,
                                    bubble);


                g_signal_handlers_disconnect_matched( G_OBJECT(bubble),G_SIGNAL_MATCH_DATA,0,0,NULL,NULL,monkey_view);
                next = g_list_next(next);
        }
}

void monkey_view_draw_lost(MonkeyView *d) {
        g_assert(IS_MONKEY_VIEW(d));

        monkey_canvas_add_block( PRIVATE(d)->canvas,
                              PRIVATE(d)->monkeys_layer,
                              PRIVATE(d)->lost,
                              320,240);
}

static void monkey_view_add_explode_stars(MonkeyView * d,Bubble * b) {
        Star * star;
        int i;
        gdouble dx,dy;

        for(i= 0; i < 2; i++) {
                star = g_malloc( sizeof(Star));
                bubble_get_position( b, &dx,&dy);

                
                star->vx = (rand()%400)/20 - 10;
                star->vy = (rand()%200)/ 10 - 15 ;
                //0.1;

                star->block = monkey_canvas_create_block_from_image( PRIVATE(d)->canvas,
                                                                  DATADIR"/monkey-bubble/gfx/star.svg",
                                                                  16,16,
                                                                  8,8);

                monkey_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->star_layer,
                                      star->block,
                                      dx,dy);

                PRIVATE(d)->star_list = g_list_append( PRIVATE(d)->star_list,
                                                       star);
        }
        
}

static Block * monkey_view_create_gem(MonkeyView * d) {

        Block * block;  
    
        block = monkey_canvas_create_block_from_image(PRIVATE(d)->canvas,
                                                   DATADIR"/monkey-bubble/gfx/banana.svg",
                                                   32,32,
                                                   8,8);
  
        return block;
  
}

static void monkey_view_clear_gems(MonkeyView * d) {
        Block * block;
        GList * next;

        next = PRIVATE(d)->gems_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                monkey_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                monkey_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->gems_list);
        PRIVATE(d)->gems_list = NULL;

}

void monkey_view_set_gems_count(MonkeyView * d,int gems) {
        Block * block;
        g_assert(IS_MONKEY_VIEW(d));
  
        monkey_view_clear_gems(d);


        while(gems > 0 ) {
		block = monkey_view_create_gem(d);
		monkey_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->shooter_layer,
                                      block,
                                      40,200+(gems*30));
  
		PRIVATE(d)->gems_list = g_list_append( PRIVATE(d)->gems_list,block);
		gems--;
        }

}


static Block * monkey_view_create_number(MonkeyView * view,int number) {
        gchar path[4096];
        gint str_length;
        Block * block;


        strcpy( path ,
                DATADIR"/monkey-bubble/gfx/number/");
	
        str_length = strlen(path);

    
        sprintf(path+str_length ,"%d.svg",number);

        block = monkey_canvas_create_block_from_image(PRIVATE(view)->canvas,
                                                   path,
                                                   32,32,
                                                   16,16);
  
        return block;

}

static void monkey_view_clear_score(MonkeyView * d) {

        Block * block;
        GList * next;

        next = PRIVATE(d)->score_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                monkey_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                monkey_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->score_list);
        PRIVATE(d)->score_list = NULL;
}


void monkey_view_set_score(MonkeyView * d,int score) {

        Block * block;
        int pre_score;
        int i = 0;
  
    
        g_assert(IS_MONKEY_VIEW(d));
        monkey_view_clear_score(d);



        if( score != 0 ) {

                while( score != 0 ) {
                        pre_score = score % 10 ;

                        block = monkey_view_create_number(d,pre_score);  
                        monkey_canvas_add_block( PRIVATE(d)->canvas,
                                              PRIVATE(d)->shooter_layer,
                                              block,
                                              450-i*20,415);

                        PRIVATE(d)->score_list = g_list_append( PRIVATE(d)->score_list,block);

                        score = score / 10;
                        i++;
                }
        } else {
                block = monkey_view_create_number(d,0);  
                monkey_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->shooter_layer,
                                      block,
                                      450,415);

                PRIVATE(d)->score_list = g_list_append( PRIVATE(d)->score_list,block);

        }

}


static void monkey_view_clear_points(MonkeyView * d) {

        Block * block;
        GList * next;

        next = PRIVATE(d)->points_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                monkey_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                monkey_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->points_list);
        PRIVATE(d)->points_list = NULL;
}


void monkey_view_set_points(MonkeyView * d,int score) {

        Block * block;
        int pre_score;
        int i = 0;
  
    
        g_assert(IS_MONKEY_VIEW(d));
        monkey_view_clear_points(d);



        if( score != 0 ) {

                while( score != 0 ) {
                        pre_score = score % 10 ;

                        block = monkey_view_create_number(d,pre_score);  
                        monkey_canvas_add_block( PRIVATE(d)->canvas,
                                              PRIVATE(d)->shooter_layer,
                                              block,
                                              580-i*20,40);

                        PRIVATE(d)->points_list = g_list_append( PRIVATE(d)->points_list,block);

                        score = score / 10;
                        i++;
                }
        } else {
                block = monkey_view_create_number(d,0);  
                monkey_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->shooter_layer,
                                      block,
                                      580,40);

                PRIVATE(d)->points_list = g_list_append( PRIVATE(d)->points_list,block);

        }

}



static Block * monkey_view_create_little_waiting(MonkeyView * d) {

        Block * block;  
    
        block = monkey_canvas_create_block_from_image(PRIVATE(d)->canvas,
                                                   DATADIR"/monkey-bubble/gfx/banana.svg",
                                                   32,32,
                                                   16,16);
  
        return block;
  
}


static Block * monkey_view_create_big_waiting(MonkeyView * d) {

        Block * block;  
    
        block = monkey_canvas_create_block_from_image(PRIVATE(d)->canvas,
                                                   DATADIR"/monkey-bubble/gfx/tomato.svg",
                                                   32,32,
                                                   16,16);
  
        return block;
  
}

static void monkey_view_clear_waiting(MonkeyView * d) {
        Block * block;
        GList * next;

        next = PRIVATE(d)->waiting_list; 
	 
        while( next != NULL ) {
		  
                block = (Block *)next->data;
 
                monkey_canvas_remove_block( PRIVATE(d)->canvas,
                                         block);
		  
                monkey_canvas_unref_block( PRIVATE(d)->canvas,
                                        block);

                next = g_list_next(next);
        }

        g_list_free( PRIVATE(d)->waiting_list);
        PRIVATE(d)->waiting_list = NULL;
}

void monkey_view_set_waiting_bubbles(MonkeyView * d,int bubbles) {
        Block * block;
        int i;
        g_assert(IS_MONKEY_VIEW(d));
  
        monkey_view_clear_waiting(d);
  
        i = 0;
        while(bubbles > 0 ) {
		if( bubbles >= 5 ) {

                        bubbles = bubbles - 5;

                        block = monkey_view_create_big_waiting(d);
		} else {
                        bubbles--;

                        block = monkey_view_create_little_waiting(d);
		}


		monkey_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->bubble_layer,
                                      block,
                                      185,400-(i*25));

		
		PRIVATE(d)->waiting_list = g_list_append( PRIVATE(d)->waiting_list,block);
		i++;
        }


}


void monkey_view_draw_win(MonkeyView *d) {
        g_assert(IS_MONKEY_VIEW(d));
        monkey_canvas_add_block( PRIVATE(d)->canvas,
                              PRIVATE(d)->monkeys_layer,
                              PRIVATE(d)->win,
                              320,240);
}


static void monkey_view_bubbles_inserted(
				      Board * board,
				      Bubble ** bubbles,
				      int count,
				      MonkeyView *d) {

        int i;
        gdouble x,y;
        Block * block;
        Bubble * b;

        for(i = 0; i < count; i++) {
                b = bubbles[i];
                bubble_get_position(b,&x,&y);
      
                block = monkey_view_create_bubble(d,
                                               b);
      
                g_hash_table_insert( PRIVATE(d)->hash_map, 
                                     b, 
                                     block );
                monkey_canvas_add_block( PRIVATE(d)->canvas,
                                      PRIVATE(d)->bubble_layer,
                                      block,
                                      x,y);

                g_signal_connect( G_OBJECT(b),"bubble-changed",G_CALLBACK(monkey_view_bubble_changed),d);
      
        }
}

static void monkey_view_bubbles_added(Board * board,
				   GList * bubbles,
				   MonkeyView *d) {
    
        gdouble x,y;
        Block * block;
        GList * next;
        BubbleAdded * added;
        Bubble * b;

        next = bubbles;

        while( next != NULL ) {
                b = (Bubble *) next->data;

                bubble_get_position(b,&x,&y);
                block = monkey_view_create_bubble(d,
                                               b);
    
                monkey_canvas_add_block( PRIVATE(d)->canvas,
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





static void monkey_view_bubbles_waiting_changed(Monkey * monkey,
					     int bubbles_count,
					     MonkeyView * view) {

        monkey_view_set_waiting_bubbles(view,bubbles_count);
}
