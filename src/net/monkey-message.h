/* monkey-message.h - 
 * Copyright (C) 2002 Christophe Segui
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
 
 #ifndef _MONKEY_MESSAGE_
#define _MONKEY_MESSAGE_

#define MAX_PLAYERS 100
#define MAX_BUBBLE 40
#define GAME_TIMEOUT 20

// 10-19 reserverd
enum ClientHandlerStateCode {
		PLAYER_NOT_READY 			=10,
		PLAYER_READY					=11,
		PLAYER_SYNCHRONIZED		=12,
		PLAYER_GAMING					=13,
		PLAYER_LOST						=14, 		
		PLAYER_WIN						=15, 
};			
// 20-39 reserved
enum PlayerMessageCode {
	// events from player's ui transmited to server's model
	// all game events come with timestamp, no ACKs except PLAYER_WANTS_TO_QUIT
	SHOOT								=20, 
	MOVE_LEFT_PRESSED  			=21,
	MOVE_LEFT_RELEASED 		=22,
	MOVE_RIGHT_PRESSED 		=23,
	MOVE_RIGHT_RELEASED 		=24,	
	PLAYER_WANTS_TO_QUIT		=25,
	PLAYER_WANTS_PAUSE		=26
};
// 40-59 reserved
enum GameMessageCode {
	ACK 			 							=40,
	SYNC 			 						=41, 
	BEGIN_GAME 							=42, 
	CLIENT_MUST_QUIT	 				=43, 
	GAME_FULL    						=44, 
	PLAYER_LIST	   						=45,
	START_GAME 							=46,
	BUBBLE_FALLEN						=47,	
	PROCESS_SYNCHRONIZATION	=48,
	CLIENT_ID								=49,
	NEXT_BUBBLE_TO_SHOOT			=50
};
// negative reserved
enum ErrorMessageCode {
	NACK = -1
};
typedef struct _positionMove{
	unsigned long x_pos;
	unsigned long y_pos;
} PositionMove;

typedef struct _positionShoot{
	unsigned long x_pos;
	unsigned long y_pos;
	unsigned int bubble_number;
} PositionShoot;

typedef struct _bubbleFallen {
	unsigned short number;
	unsigned short colors[MAX_BUBBLE];
	//the bubbles...
}BubbleFallen;

typedef struct _monkeyMessage {
	unsigned int from;
	unsigned short message;
	gint time_stamp;	
	union {
		struct _positionMove pos_move;
		struct _positionShoot pos_shoot;
		struct _bubbleFallen	bubble_fallen;
		unsigned long players[MAX_PLAYERS];
		unsigned long id;
		unsigned short color;
	} arg;
} MonkeyMessage;

#endif

