/* board.c
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
#include "board.h"
#include "point.h"

#define PRIVATE(board) (board->private)
#define QUADRANT_INITIAL  0
#define QUADRANT_END  7
#define BUBBLE_SIZE 32
#define ROW_SIZE ((BUBBLE_SIZE * 7)/8)
#define COLUMN_COUNT 8
#define ROW_COUNT 13
#define BUBBLE_RADIUS 16
static GObjectClass* parent_class = NULL;

static void board_get_bubble_position(Board * board,
				      gint cell_x,gint cell_y,
				      PointDouble * point);

static gint advance_quadrant(Board *board,
			     gint quadrant,
			     Point * point);

static Bubble * board_get_bubble(Board * board,
				 gint cell_x,
				 gint cell_y) ;

static void board_bubble_added(Board * board,Bubble * b);
static void board_bubble_removed(Board * board,Bubble * b);

static void board_load_from_file(Board * board,
				 const char * filename,
				 gint level);

static void board_set_bubble(Board * board,
			     Bubble * b,
			     gint cell_x,
			     gint cell_y);

static void board_get_cell(Board * board,
			   gdouble x,gdouble y,
			   Point * point);

static int recursive_tag_exploded(Board * board,
				  gint cell_x,
				  gint cell_y,
				  Color color);

static GList * board_get_exploded(Board * board, 
				  gint cell_x,
				  gint cell_y);

static GList * board_get_fallen(Board * board);

static void recursive_tag_fallen(Board * board,
				 gint cell_x,
				 gint cell_y);

static void board_notify_bubbles_exploded(Board * board,
					  GList * exploded,
					  GList * fallen);

static void board_notify_bubble_sticked(Board * board,
					Bubble * bubble,
					int time);

static void board_notify_bubbles_added(Board * board,
				       GList * bubbles);

static void board_notify_bubbles_inserted(Board * board,
					  Bubble ** bubbles,
					  int count);

static void board_notify_down(Board * board);

struct BoardPrivate {
  gdouble y_min;
  gint min_row;
  Bubble ** bubble_array;
  GList * observer_list;
  gint odd;
  gdouble x_center;
  gdouble y_center;
  gint tag_array[ROW_COUNT*COLUMN_COUNT];
  int colors[COLORS_COUNT];
};


static void board_instance_init(Board * board) {
  board->private =g_new0 (BoardPrivate, 1);			
}

static void board_finalize(GObject* object) {
  int i;
  Board * board = BOARD(object);

  if(PRIVATE(board)->observer_list != NULL) {
    g_error("[Board] All observer has not been removed");		
  }

  for( i= 0; i< ROW_COUNT*COLUMN_COUNT;i++) {
    
    if( PRIVATE(board)->bubble_array[i] != NULL ) {
      g_object_unref( PRIVATE(board)->bubble_array[i]);
    }
  }

  g_free( PRIVATE(board)->bubble_array);
  g_free( PRIVATE(board) );

  if (G_OBJECT_CLASS (parent_class)->finalize) {
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
  }
}


static void board_class_init (BoardClass *klass) {
  GObjectClass* object_class;
    
  parent_class = g_type_class_peek_parent(klass);
  object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = board_finalize;
}


GType board_get_type(void) {
  static GType board_type = 0;
    
  if (!board_type) {
    static const GTypeInfo board_info = {
      sizeof(BoardClass),
      NULL,           /* base_init */
      NULL,           /* base_finalize */
      (GClassInitFunc) board_class_init,
      NULL,           /* class_finalize */
      NULL,           /* class_data */
      sizeof(Board),
      1,              /* n_preallocs */
      (GInstanceInitFunc) board_instance_init,
    };


      
    board_type = g_type_register_static(G_TYPE_OBJECT,
					"Board",
					&board_info, 0);
  }
    
  return board_type;
}


Board * board_new(gdouble y_min ,const gchar * level_filename,gint level) {

  Board * b;
  int i;
  b =BOARD( g_object_new(TYPE_BOARD,NULL));

  PRIVATE(b)->y_min = y_min;

  PRIVATE(b)->observer_list = NULL;
  PRIVATE(b)->bubble_array = 
    g_malloc( sizeof( Bubble * )*ROW_COUNT*COLUMN_COUNT);
  
  PRIVATE(b)->min_row = 0;
  PRIVATE(b)->odd = 1;
  PRIVATE(b)->x_center = 190;
  PRIVATE(b)->y_center = y_min;

  for( i = 0 ; i < COLORS_COUNT; i++ ) {
    PRIVATE(b)->colors[i] = 0;
  }

  for( i = 0 ; i < ROW_COUNT*COLUMN_COUNT ; i++ ) {
    PRIVATE(b)->bubble_array[i] = NULL;
  }

  if( level_filename != NULL) {
    board_load_from_file( b,level_filename,level);
  }

  return b;
}

void board_load_from_file(Board * board,
			  const char * filename,
			  gint level) {

  GError * error;
  GIOChannel * channel;
  gsize length;
  PointDouble point;
  gint n,i;
  Bubble * bubble;
  gchar * line;
  error = NULL;
  
  channel = g_io_channel_new_file(filename,
				  "r",&error);

  n = 0;
  
  while(level > 0 ) {
	
	 g_io_channel_read_line( channel,
								  &line,
								  &length,
								  NULL,
								  &error);
 	
		while( length > 2 )  {
			 
			 g_free(line);
			 g_io_channel_read_line( channel,
											 &line,
											 &length,
											 NULL,
											 &error);
			 
		}
  
		level--;
  }
  
  g_io_channel_read_line( channel,
								  &line,
								  &length,
								  NULL,
								  &error);
 
  n = 0;
  while( length > 2 )  {

    for(i = 0; i < (8 - ( n%2));i++) {
      if( line[i*4 + ( n%2)*2 ] != '-') {
	board_get_bubble_position( board,i, n,&point);

	bubble = bubble_new( (Color)line[ i*4 + (n%2)*2 ] - '0' ,
			     point.x,point.y);
	board_bubble_added(board,bubble);
	PRIVATE(board)->bubble_array[ i + n*COLUMN_COUNT ] = bubble;
      } else {
      }
    }

    g_free(line);
    g_io_channel_read_line( channel,
			    &line,
			    &length,
			    NULL,
			    &error);
    n++;

  } 
  
  g_io_channel_shutdown(channel,
			TRUE,
			&error);
  g_io_channel_unref( channel );
  
}


gdouble board_get_y_min( Board * board) {
  g_assert( IS_BOARD(board));

  return PRIVATE(board)->y_min; 
}

static void board_get_bubble_position(Board * board,
				      gint cell_x,gint cell_y,
				      PointDouble * point) {

  
  point->x = cell_x * BUBBLE_SIZE + 
    BUBBLE_RADIUS  +
    // decalage selon ligne pair ou impare
    // TODO : FIXME comment translation
    BUBBLE_RADIUS*((  cell_y + 1 + PRIVATE(board)->odd)%2 ) + 
    PRIVATE(board)->x_center;
  
  point->y = cell_y * ROW_SIZE 
    + BUBBLE_RADIUS 
    + PRIVATE(board)->y_center;
  
}

static void board_get_cell(Board * board,
			   gdouble x,gdouble y,
			   Point * point) {

  point->y = 
    (gint) ( y - PRIVATE(board)->y_center ) / ROW_SIZE;
    

  point->x = 
    ( 
     x - PRIVATE(board)->x_center 
     - BUBBLE_RADIUS*(  ( 1 + point->y + PRIVATE(board)->odd  ) %2 ))
    / BUBBLE_SIZE;
  
}


Bubble * board_get_bubble_at(Board * board,gint x,gint y) {
  g_assert( IS_BOARD( board));
	    
  if( (x >= 0) && ( x < COLUMN_COUNT)  && 
      ( y >= 0 ) && ( y < ROW_COUNT) ) {
    return  PRIVATE(board)->bubble_array[x + y*COLUMN_COUNT];
  } else {
    return NULL;
  }
	    
}

gint board_get_row_count(Board * board) {
 
  return ROW_COUNT;
}

gint board_get_column_count(Board *board) {
  return COLUMN_COUNT;
}

static void board_bubble_added(Board * board,Bubble * b) {
  PRIVATE(board)->colors[bubble_get_color(b)]++;
}

static void board_bubble_removed(Board * board,Bubble * b) {
  PRIVATE(board)->colors[bubble_get_color(b)]--;
}

gint * board_get_colors_count(Board * board) {
  return PRIVATE(board)->colors;
}

void board_stick_bubble(Board *board,Bubble *bubble,gint time) {
    
  PointDouble position;
  Point cell;
  gdouble x,y;

  GList * exploded;
  GList * fallen;
  GList * next;
  g_assert( IS_BOARD(board));
  g_assert( IS_BUBBLE(bubble));
  
  bubble_get_position( bubble ,&x,&y);

  board_get_cell(board,
		 x,y,
		 &cell);


  board_get_bubble_position(board,
			    cell.x,cell.y,
			    &position);
  
  
  bubble_set_position( bubble, position.x, position.y);


  board_set_bubble( board, bubble, cell.x,cell.y );
  // get exploded and fallen bubbles
  exploded = board_get_exploded( board, cell.x,cell.y);
  
  if( exploded != NULL ) {
    fallen = board_get_fallen(board);
  } else {
    fallen = NULL;
  }
  
 
  if( exploded != NULL ) {
    board_notify_bubbles_exploded(board,exploded,fallen);

    next = exploded;
    while( next != NULL ) {
      bubble = BUBBLE(next->data);
      g_object_unref(bubble);
      next = g_list_next(next);
    }

    next = fallen;
    while( next != NULL ) {
      bubble = BUBBLE(next->data);
      g_object_unref(bubble);
      next = g_list_next(next);
    }

    g_list_free(exploded);
    g_list_free(fallen);
  }

  board_notify_bubble_sticked(board,bubble,time);

}


static GList * board_get_fallen(Board * board) {
  gint i;
  GList * fallen_list;

  // clear tag array
  for(i = 0 ; i < COLUMN_COUNT*ROW_COUNT; i++ ) {
    PRIVATE(board)->tag_array[i] = 0;    
  }

  for(i = 0; i < COLUMN_COUNT; i++) {
    recursive_tag_fallen(board,
			 i,PRIVATE(board)->min_row);
  }

  fallen_list = NULL;

  for(i = 0 ; i < COLUMN_COUNT*ROW_COUNT; i++ ) {
    if( ( PRIVATE(board)->tag_array[i] == 0) &&
	( PRIVATE(board)->bubble_array[i]!= NULL ) ) {
      
      fallen_list =
	g_list_append( fallen_list,
		       PRIVATE(board)->bubble_array[i]);

      board_bubble_removed(board,PRIVATE(board)->bubble_array[i]);
      PRIVATE(board)->bubble_array[i] = NULL;
		     
    }
  }

  return fallen_list;
}

void board_down(Board * board) {
  Bubble * b;
  int i,j;
  PointDouble p;

  g_assert( IS_BOARD( board) );

  PRIVATE(board)->odd++;
  for(j = ROW_COUNT-2; j >= 0; j--) {
    
    for(i = 0; i < COLUMN_COUNT; i++) {
      
      b = PRIVATE(board)->bubble_array[i + ( j* COLUMN_COUNT) ];
      PRIVATE(board)->bubble_array[i + ( j* COLUMN_COUNT) ] = NULL;
      
      PRIVATE(board)->bubble_array[i + ((j+1)*COLUMN_COUNT) ] = b;
      
      if( b != NULL ) {
	board_get_bubble_position( board, i,j+1,&p);
	bubble_set_position( b,p.x,p.y);
      }
    }
    
  }

  PRIVATE( board )->y_min+= ROW_SIZE;

  PRIVATE( board )->min_row++;

  board_notify_down(board);
}

void board_attach_observer(Board * board,IBoardObserver * bo) {
  g_assert(IS_BOARD(board));
  g_assert(IS_IBOARD_OBSERVER(bo));

  PRIVATE(board)->observer_list = 
    g_list_append(PRIVATE(board)->observer_list, bo);
}

void board_detach_observer(Board * board,IBoardObserver * bo) {
  g_assert(IS_BOARD(board));
  g_assert(IS_IBOARD_OBSERVER(bo));

  PRIVATE(board)->observer_list = 
    g_list_remove(PRIVATE(board)->observer_list,bo);
}


static void recursive_tag_fallen(Board * board,
				 gint cell_x,
				 gint cell_y) {

  Point cell;

  gint quadrant;

  Bubble * bubble;

  cell.x = cell_x;
  cell.y = cell_y;

  bubble = board_get_bubble( board,cell.x,cell.y);


  if( (bubble != NULL) &&
      ( PRIVATE(board)->tag_array[cell.x+cell.y*COLUMN_COUNT] == 0)) {
    
    PRIVATE(board)->tag_array[cell.x + cell.y*COLUMN_COUNT ] = 1;

    quadrant = QUADRANT_INITIAL;
    
    do {
      
      quadrant = advance_quadrant( board,
				   quadrant,
				   &cell);

      if( (cell.x < COLUMN_COUNT ) && (cell.x >= 0 ) &&
	  (cell.y < ROW_COUNT ) && (cell.y >= 0 ) &&
	  (PRIVATE(board)->tag_array[cell.x + cell.y*COLUMN_COUNT] == 0) ) {

	recursive_tag_fallen( board,
			      cell.x,
			      cell.y);	
      }
    }while(  quadrant < QUADRANT_END  );

    
  } else {
    PRIVATE(board)->tag_array[cell.x + cell.y*COLUMN_COUNT ] = 1;
  }
    
  
  
}

static GList * board_get_exploded(Board * board, 
				  gint cell_x,
				  gint cell_y) {

  gint i;
  gint color;
  gint exploded_count;
  GList * exploded_list;

  // clear tag array
  // TODO : make a method to clear the tag array
  for(i = 0 ; i < COLUMN_COUNT*ROW_COUNT; i++ ) {
    PRIVATE(board)->tag_array[i] = 0;    
  }

  color = bubble_get_color( board_get_bubble( board,
					      cell_x,
					      cell_y));
  exploded_count =
    recursive_tag_exploded( board,
			    cell_x,cell_y,
			    color);

  exploded_list = NULL;

  if( exploded_count >=3 ) {

    for( i = 0; i <COLUMN_COUNT*ROW_COUNT; i++ ) {
      if( PRIVATE(board)->tag_array[i] == 2) {
	exploded_list =
	  g_list_append( exploded_list,
			 PRIVATE(board)->bubble_array[i]);
	
	board_bubble_removed(board,PRIVATE(board)->bubble_array[i]);
	PRIVATE(board)->bubble_array[i] = NULL;

      }
    
    }
    
    return exploded_list;
  } else {    
    return NULL;
  }
  
}

static gint recursive_tag_exploded(Board * board,
				   gint cell_x,
				   gint cell_y,
				   Color color) {
  
  gint quadrant;
  gint count;
  Point cell;
  Bubble * bubble;

  cell.x = cell_x;
  cell.y = cell_y;

  bubble = board_get_bubble( board,cell.x,cell.y);

  if( (bubble != NULL) &&
      (color == bubble_get_color( bubble ))) {
    
    PRIVATE(board)->tag_array[cell.x + cell.y*COLUMN_COUNT ] = 2;
    count = 1;
    quadrant = QUADRANT_INITIAL;
    
    do {
      
      quadrant = advance_quadrant( board,
				   quadrant,
				   &cell);

      if( (cell.x < COLUMN_COUNT ) && (cell.x >= 0 ) &&
	  (cell.y < ROW_COUNT ) && (cell.y >= 0 ) &&
	  (PRIVATE(board)->tag_array[cell.x + cell.y*COLUMN_COUNT] == 0) ) {

	count += recursive_tag_exploded( board,
					 cell.x,
					 cell.y,
					 color);	
      }
    }while(  quadrant < QUADRANT_END  );

    return count;
    
  } else {
    PRIVATE(board)->tag_array[cell.x + cell.y*COLUMN_COUNT ] = 1;
    return 0;
  }
    
  
  
}

static gint advance_quadrant(Board *board,
			     gint quadrant,
			     Point * cell) {
  gint x,y;
  x=cell->x;
  y=cell->y;
  switch(quadrant) {
  case QUADRANT_INITIAL :
    cell->x++;
    return 1;
    break;
  case 1 :
    if( ( (1+y + PRIVATE(board)->odd) % 2) == 0) {
      cell->x--;
    }

    cell->y++;
    return 2;
    break;


  case 2:
    cell->x--;
    return 3;
      
      
  case 3:
    if( ( ( 1+y + PRIVATE(board)->odd ) %2) == 0 ) {
      cell->x--;
    }
    cell->y--;
    return 4;
    break;
  case 4:
    if (( (1+y+ PRIVATE(board)->odd )%2) != 0 ) {
      cell->x++;
    }    
    cell->y--;
    return 5;
    break;
  case 5:
    cell->x++;
    return 6;
    break;
  case 6:    
    return QUADRANT_END;
    break;
  }

  return QUADRANT_END;
}


int board_bubbles_count(Board * board) {

	 int i,count;
	 g_assert( IS_BOARD(board));

	 count = 0;
	 
	 for( i = 0; i < ROW_COUNT*COLUMN_COUNT;i++) {
		  if( PRIVATE(board)->bubble_array[ i ] != NULL ) {

				count++;
		  }
	 }

	 return count;
}

static Bubble * board_get_bubble(Board * board,
				 gint cell_x,
				 gint cell_y) {

  if( ( cell_x < COLUMN_COUNT) && ( cell_x >= 0 ) &&
      ( cell_y < ROW_COUNT ) && ( cell_y >= 0 ) ) {

    return PRIVATE(board)->bubble_array[ cell_x + cell_y*COLUMN_COUNT ];
  } else {
    return NULL;
  }
}


static void board_set_bubble(Board * board,
			     Bubble * b,
			     gint cell_x,
			     gint cell_y) {


  if( ( cell_x < COLUMN_COUNT) && ( cell_x >= 0 ) &&
      ( cell_y < ROW_COUNT ) && ( cell_y >= 0 ) ) {

    if(  PRIVATE(board)->bubble_array[ cell_x + cell_y*COLUMN_COUNT ]
	 == NULL)  {

      board_bubble_added(board,b);
      PRIVATE(board)->bubble_array[ cell_x 
				    + cell_y*COLUMN_COUNT ] = b;
    } else {
    
      g_error("bubble already present in %d %d",
	      cell_x,cell_y);
    }
  } else {
		//  g_error(" invalid coordonnate %d,%d",cell_x,cell_y);
  }
}


gboolean board_collide_bubble(Board * board,Bubble *b) {
  gdouble x,y;
  Point cell;
  gint quadrant;
  gboolean collide;
  Bubble * bubble;
  g_assert( IS_BOARD(board));
  g_assert( IS_BUBBLE(b));
  
  collide = FALSE;
  
  bubble_get_position(b,&x,&y);

  board_get_cell(board,
		 x,y,&cell);





  quadrant = QUADRANT_INITIAL;
    
  do {
    bubble = board_get_bubble(board,
			      cell.x,cell.y);

    if( bubble != NULL ) {
      collide = bubble_collide_bubble( b, bubble );
    }
  
    
    quadrant = advance_quadrant( board,
				 quadrant,
				 &cell);
    
  }while( !collide && ( quadrant < QUADRANT_END ) );

  
  if( (y < ( PRIVATE(board)->y_min +16) ) ||
      collide ) {
    return TRUE;
    
  } else {
    return FALSE;
  }
  
}

void board_init(Board * board,Bubble ** bubbles,gint count) {
  gint i;
  gint cell_x,cell_y;
  PointDouble p;

  g_assert( IS_BOARD( board ));
 
  cell_x = 0;
  cell_y = 0;
  for( i = 0; i < count; i++) {

    board_set_bubble( board,
		      bubbles[i],
		      cell_x,cell_y);

    board_get_bubble_position( board, cell_x,cell_y,&p);
    bubble_set_position( bubbles[i],p.x,p.y);
    
    cell_x++;
    if( cell_x >= ( COLUMN_COUNT - ( cell_y + PRIVATE(board)->odd  + 1)%2 )) {
      cell_y++;  
      cell_x = 0;
    }
    
  }

}

gboolean board_is_lost(Board * board) {
  int x;
  gboolean lost;
  
  g_assert( IS_BOARD(board) );

  lost = FALSE;
  
  for( x=0; ( x <COLUMN_COUNT) && ( !lost) ; x++) {
    
    if( PRIVATE(board)->bubble_array[ x + (ROW_COUNT-1)*(COLUMN_COUNT) ] 
	!= NULL ) {
      lost = TRUE;
    }
  }
  
  return lost;
}

void board_add_bubbles(Board *board,
							  Bubble ** bubbles ) {

  int i,column;
  gboolean sticked;
  int y_cell;
  PointDouble position;
  GList * bubbles_list;
  Bubble * bubble;
  g_assert( IS_BOARD( board) );
    
  bubbles_list = NULL;
  for( i = 0; i<  7; i++) {
    if( bubbles[i] != NULL) {
      column = i;
      y_cell = ROW_COUNT - 1 ;

      bubble = bubbles[i];
      bubbles_list = g_list_append(bubbles_list,bubble);
      sticked = FALSE;

      do {
	    
	if( PRIVATE(board)->bubble_array[column + y_cell*COLUMN_COUNT ] 
	    != NULL ) {

	  y_cell++;
	  board_get_bubble_position(board,column,y_cell,&position);
	  bubble_set_position(bubble,position.x,position.y);
	  board_set_bubble(board,bubble,column,y_cell);	
	  sticked = TRUE;
	} else {
	  
	  if( y_cell == 0 ) {
	    board_get_bubble_position(board,column,y_cell,&position);
	    bubble_set_position(bubble,position.x,position.y);
	    board_set_bubble(board,bubble,column,y_cell);	
	    sticked = TRUE;
	  }  else {
	    y_cell--;
	  }
	}
	
      } while( ! sticked );
    }
  }


  g_free( bubbles);
  board_notify_bubbles_added(board,bubbles_list);
  
  g_list_free(bubbles_list);

}



static void board_notify_bubbles_exploded(Board * board,
					  GList * exploded,
					  GList * fallen) {
  GList * next;
  IBoardObserver * bo;

  next = PRIVATE(board)->observer_list;

  while( next != NULL ) {
    bo = IBOARD_OBSERVER( next->data );
    iboard_observer_bubbles_exploded(bo,board,exploded,fallen);

    next = g_list_next(next);
  }
    
}

void board_insert_bubbles(Board *board,
			  Bubble ** bubbles) {

  Bubble * b;
  int i,j,max;
  PointDouble p;
    
  g_assert( IS_BOARD( board ) );


  PRIVATE(board)->odd++;

  for(j = ROW_COUNT-2; j >= 0; j--) {

    for(i = 0; i < COLUMN_COUNT; i++) {

      b = PRIVATE(board)->bubble_array[i + ( j* COLUMN_COUNT) ];
      PRIVATE(board)->bubble_array[i + ( j* COLUMN_COUNT) ] = NULL;

      PRIVATE(board)->bubble_array[i + ((j+1)*COLUMN_COUNT) ] = b;

      if( b != NULL ) {
	board_get_bubble_position( board, i,j+1,&p);
	bubble_set_position( b,p.x,p.y);
      }
    }

  }

  if( (PRIVATE(board)->odd %2) != 0) {
    max = 8;
  } else {
    max = 7;
  }
  
  for(i = 0 ; i< max; i++) {
    b =bubbles[i];
    PRIVATE(board)->bubble_array[i] = b;
    board_bubble_added(board,b);
    board_get_bubble_position( board, i,0,&p);
    bubble_set_position( b,p.x,p.y);
  }

  board_notify_bubbles_inserted(board,bubbles,max);
}

static void board_notify_bubble_sticked(Board * board,
					Bubble * bubble,
					gint time) {

  GList * next;
  IBoardObserver * bo;

  next = PRIVATE(board)->observer_list;

  while( next != NULL ) {
    bo = IBOARD_OBSERVER( next->data );
    iboard_observer_bubble_sticked(bo,board,bubble,
				   time);

    next = g_list_next(next);
  }
    
}


static void board_notify_bubbles_added(Board * board,
				       GList * bubbles) {

  GList * next;
  IBoardObserver * bo;

  next = PRIVATE(board)->observer_list;

  while( next != NULL ) {
    bo = IBOARD_OBSERVER( next->data );
    iboard_observer_bubbles_added(bo,board,bubbles);

    next = g_list_next(next);
  }
    
}


static void board_notify_down(Board * board) {

  GList * next;
  IBoardObserver * bo;
  
  next = PRIVATE(board)->observer_list;

  while( next != NULL ) {
    bo = IBOARD_OBSERVER( next->data );
    iboard_observer_down(bo,board);

    next = g_list_next(next);
  }
    
}

static void board_notify_bubbles_inserted(Board * board,
					  Bubble ** bubbles,
					  int count) {

  GList * next;
  IBoardObserver * bo;

  next = PRIVATE(board)->observer_list;

  while( next != NULL ) {
    bo = IBOARD_OBSERVER( next->data );
    iboard_observer_bubbles_inserted(bo,board,bubbles,count);

    next = g_list_next(next);
  }
    
}
