/* monkey-client.c - 
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
 
#include "monkey-client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <gtk/gtk.h>



#define PRIVATE( monkeyClient ) (monkeyClient->private)

static GObjectClass* parent_class = NULL;


gboolean read_update_from_monkey_server(MonkeyMessage *, MonkeyClient *) ;
gboolean process_message(MonkeyMessage *, MonkeyClient *);
gboolean process_time_sync(MonkeyClient *);
void *start(MonkeyClient *);
void monkey_client_finalize(GObject *);
void update_player_list(MonkeyMessage *, MonkeyClient *);
gboolean update_local_monkey_image(MonkeyMessage *, MonkeyClient *);
gboolean process_game_initialization(MonkeyClient *);
void *finalize_image_of_foreign_client(gpointer key, gpointer value, gpointer user_data);

struct MonkeyClientPrivate {
	int socket;
	GThread *thr;
	gboolean is_running;
	NetworkGame *network_game;
	GHashTable *foreign_monkeys;
	gint id;
	unsigned long game_offset;
};



MonkeyClient *monkey_client_new(NetworkGame *ng) {
	GError *err;
	MonkeyClient *mc;
	mc = MONKEY_CLIENT(g_object_new(TYPE_MONKEY_CLIENT, NULL));
	err = NULL;
	
	g_assert(IS_MONKEY_CLIENT(mc));
	
	PRIVATE(mc)->network_game = ng;
	PRIVATE(mc)->thr = NULL;	
	PRIVATE(mc)->is_running = TRUE;
	PRIVATE(mc)->foreign_monkeys = g_hash_table_new(g_direct_hash,g_direct_equal);

	return(mc);
}

void monkey_client_finalize(GObject *object) {
	
	MonkeyClient *mc = MONKEY_CLIENT(object);
	MonkeyMessage *m = (struct _monkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
	

	PRIVATE(mc)->is_running = FALSE;

	g_hash_table_foreach(PRIVATE(mc)->foreign_monkeys, (GHFunc )finalize_image_of_foreign_client, NULL); 

	m->message = PLAYER_WANTS_TO_QUIT;
	monkey_client_send_update_to_monkey_server(m,mc);
	close(PRIVATE(mc)->socket);	

	g_thread_join(PRIVATE(mc)->thr);	   	

	g_free(PRIVATE(mc));

 	if (G_OBJECT_CLASS (parent_class)->finalize) {
    	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
  	}
}
void *finalize_image_of_foreign_client(gpointer key, gpointer value, gpointer user_data) {
	
	g_assert(IS_MONKEY(value));		
	g_object_unref(MONKEY(value));
	return(0);
}
static void monkey_client_instance_init(MonkeyClient * mc) {
  mc->private =g_new0 (MonkeyClientPrivate, 1);			
}
static void monkey_client_class_init (MonkeyClientClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = monkey_client_finalize;
}
GType monkey_client_get_type(void) {
    static GType monkey_client_type = 0;
    
    if (!monkey_client_type) {
      static const GTypeInfo monkey_client_info = {
	sizeof(MonkeyClientClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) monkey_client_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(MonkeyClient),
	1,              /* n_preallocs */
	(GInstanceInitFunc) monkey_client_instance_init,
      };


      monkey_client_type = g_type_register_static(G_TYPE_OBJECT,
						"MonkeyClient",
						&monkey_client_info, 0);


    }
    
    return monkey_client_type;
}

gboolean monkey_client_init(unsigned short port, char *server, MonkeyClient *mc) {
	struct hostent *src_host;
	struct sockaddr_in sock_client;

	if ( (mc->private->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("socket()");
		return(FALSE);
	}	
	bzero((char *) &sock_client, sizeof(sock_client));
	sock_client.sin_family = AF_INET;
	sock_client.sin_port = (unsigned short) htons(port);
	src_host = (struct hostent *) gethostbyname(server);	
	if (!src_host)
	{
		fprintf(stderr, "Not a valid Server IP...\n");
		return(FALSE);
	}  
	bcopy( (char *) src_host->h_addr, (char *) &sock_client.sin_addr.s_addr, src_host->h_length);
	while (connect(mc->private->socket, (struct sockaddr *) &sock_client, sizeof(sock_client)) == -1)
	{
		if (errno != EAGAIN)
		{
			perror("connect()");
			return(FALSE);
		}
	}	
	return(TRUE);
}
GThread *monkey_client_get_thread(MonkeyClient *mc) {
	return(mc->private->thr);
}
void monkey_client_start(MonkeyClient *mc) {
	GThread *thr;	
	GError **err = NULL;		
	thr = g_thread_create((GThreadFunc) start, mc, TRUE, err);
	mc->private->thr =  thr;		
}
void *start(MonkeyClient *mc) {

	while (mc->private->is_running) {
		MonkeyMessage *m;	
		m = (struct _monkeyMessage*) malloc (sizeof(struct _monkeyMessage));	
		if (!read_update_from_monkey_server(m,mc)) {			
			fprintf(stderr, "Unable to get update from server\n");
			mc->private->is_running = FALSE;
		}
		else {
			if (!process_message(m,mc))
				fprintf(stderr, "Unable to process message\n");		
		}
	}
	g_print("MonkeyClient->start() : End of client exiting...\n");	
	g_thread_exit(0);
	return(0);
}

gboolean process_message(MonkeyMessage *m, MonkeyClient *mc) {

	switch(m->message) {
		case NACK :										g_print("Got Error Message\n");
					  												break;
		case ACK : 											g_print("Got ACK, no need here...\n");
													  				break; 
		case SYNC:
#ifdef _DEBUG_
																	g_print("**DEBUG** Got SYNC, no need here....\n");
#endif			 							 
					 												break;
		case PROCESS_SYNCHRONIZATION: 
#ifdef _DEBUG_
																	g_print("**DEBUG** Got PROCESS_SYNCHRONIZATION\n");					 						
#endif											
																	if (!process_game_initialization(mc)) {
																		g_print("Unable to process PROCESS_SYNCHRONIZATION\n");																
																		PRIVATE(mc)->is_running = FALSE;
																	}
																	break;
		case SHOOT: 
		case MOVE_LEFT_PRESSED:
		case MOVE_LEFT_RELEASED:
		case MOVE_RIGHT_PRESSED: 
		case MOVE_RIGHT_RELEASED: 					if (m->from !=PRIVATE(mc)->id) {
																		 if (!update_local_monkey_image(m,mc))
																			g_print("Unable to update local monkey image\n");														
																	}
							 										break;							 		
		case CLIENT_MUST_QUIT: 					PRIVATE(mc)->is_running = FALSE;
#ifdef _DEBUG_		
																	g_print("**DEBUG** MonkeyClient->process_message(): Got CLIENT_MUST_QUIT\n");
#endif				
																	break;
		case GAME_FULL: 								//prevenir le processus de dereferencer monkey-client
																	//client destruction
#ifdef _DEBUG_		
																	g_print("**DEBUG** MonkeyClient->process_message(): Got GAME_FULL\n");
#endif		
																	
																	break;
		
		case PLAYER_WANTS_TO_QUIT:  		g_print("Got CLIENT_WANTS_TO_QUIT, no need here....\n");
																	break;
		case PLAYER_LIST: 								g_print("Got PLAYER_LIST, no need here....\n");
																	break;
		case PLAYER_LOST:								
		case PLAYER_WIN:								if (m->from != PRIVATE(mc)->id)
																		update_local_monkey_image(m,mc);
																	else network_game_update_monkey(PRIVATE(mc)->network_game,m);
																	break;
		case BUBBLE_FALLEN:							//todo
																	break;
		case BEGIN_GAME:							 																																																			  
#ifdef _TIME_DEBUG_		
																	g_print("**DEBUG** MonkeyClient->process_message(): Got BEGIN_GAME, server start is %d\n",m->time_stamp);
																	g_print("**DEBUG**  MonkeyClient->process_message(): Local time is %d\n", network_game_get_time(PRIVATE(mc)->network_game));
																	 g_print("**DEBUG**  MonkeyClient->process_message(): game_offset is %d\n", m->time_stamp);
#endif															
//																	g_print("hihihi\n");
																	PRIVATE(mc)->game_offset = m->time_stamp;		
//																																		g_print("hihihi\n");
																	network_game_set_game_offset(PRIVATE(mc)->network_game, m->time_stamp);
//																																		g_print("hihihi\n");
																	game_start( (Game *) PRIVATE(mc)->network_game);
																	g_print("hihihi\n");
																	break;
		case NEXT_BUBBLE_TO_SHOOT:
#ifdef _DEBUG_		
																	g_print("**DEBUG** MonkeyClient->process_message(): Got NEXT_BUBBLE_TO_SHOOT\n");
#endif		
																	if (m->from !=PRIVATE(mc)->id) {
																		 if (!update_local_monkey_image(m,mc))
																			g_print("Unable to update local monkey image\n");														
																	}
																	else network_game_update_monkey(PRIVATE(mc)->network_game,m);
																	break;
		default : 												g_print("Got Unknown Message %d\n",m->message);
					 												break;
						 
	}
	return(TRUE);		
}

gboolean update_local_monkey_image(MonkeyMessage *mm, MonkeyClient *mc) {
	Monkey *m;
	
	if ((m = g_hash_table_lookup( PRIVATE(mc)->foreign_monkeys, GINT_TO_POINTER(mm->from))) == NULL)
		return(FALSE);
		
	mm->time_stamp -= PRIVATE(mc)->game_offset;
#ifdef _DEBUG_
	g_print("**DEBUG** MonkeyClient->update_local_monkey_image() : local copy of monkey found, updating\n");
	g_print("**DEBUG** MonkeyClient->update_local_monkey_image() : message is %d at %d\n",mm->message, mm->time_stamp);
#endif
	
	switch(mm->message) {
		case SHOOT: 							monkey_shoot(m,mm->time_stamp);
							 							break;
		case MOVE_LEFT_PRESSED: 	monkey_left_changed(m, TRUE, mm->time_stamp);
														break;
		case MOVE_LEFT_RELEASED: 	monkey_left_changed(m, FALSE, mm->time_stamp);
														break;
		case MOVE_RIGHT_PRESSED: 	monkey_right_changed(m, TRUE, mm->time_stamp);
														break;
		case MOVE_RIGHT_RELEASED: monkey_right_changed(m, FALSE, mm->time_stamp);
														break;
		case NEXT_BUBBLE_TO_SHOOT: g_print("next bubble color for displayed monkey: %d\n",mm->arg.color);
													 shooter_add_bubble(monkey_get_shooter(m),bubble_new(mm->arg.color,0,0));
													 break;
		default: g_print("MonkeyClient->update_local_monkey_image: !What are we doing here?!? (Message %d) \n", mm->message);
						break;						
	}
	return(TRUE);
}
void update_player_list(MonkeyMessage *m, MonkeyClient *mc){
	int i;
	int nb_players= ntohl(m->arg.players[0]);
	
#ifdef _DEBUG_
		g_print("**DEBUG** MonkeyClient->update_player_list: got %d Players\n",nb_players);
#endif
	for (i=1; i <nb_players+1; i++) {
		m->arg.players[i] = ntohl(m->arg.players[i]);	
#ifdef _DEBUG_
		g_print("**DEBUG**MonkeyClient->update_player_list: Examining client id %ld, number %d\n", m->arg.players[i], i);
#endif
		//except monkey of the local player
		if (m->arg.players[i] != PRIVATE(mc)->id)  {
#ifdef _DEBUG_
			g_print("**DEBUG** MonkeyClient->update_player_list: MonkeyClient->update_player_list : inserting player %ld\n", m->arg.players[i]);			
#endif			
			g_hash_table_insert( PRIVATE(mc)->foreign_monkeys, GINT_TO_POINTER((gint) m->arg.players[i]), monkey_new());		
		}		
	}
}
gboolean process_game_initialization(MonkeyClient *mc) {
		MonkeyMessage *m = (struct _monkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
		
		m->message = SYNC;
		monkey_client_send_update_to_monkey_server(m,mc);
		//get our id on server side
		if (!read_update_from_monkey_server(m,mc))
			return(FALSE);
		if (m->message != CLIENT_ID) 	
			return(FALSE);
		PRIVATE(mc)->id = ntohl(m->arg.id);		
#ifdef _DEBUG_
	g_print("**DEBUG** :MonkeyCLient->process_game_initialization() : Got id %ld.\n",(unsigned long) ntohl(m->arg.id));
#endif		
		//get player_list
		if (!read_update_from_monkey_server(m,mc))
			return(FALSE);	
#ifdef _DEBUG_
	g_print("**DEBUG** :MonkeyCLient->process_game_initialization() : Got Player.\n");
#endif						
		update_player_list(m,mc);
		// process sync
		if (!process_time_sync(mc)) {
			g_print("Unable to process clock syncronization\n");
			return(FALSE);
		}		
		m->message = ACK;
		if (!monkey_client_send_update_to_monkey_server(m,mc))
			return(FALSE);		
		g_free(m);	
		return(TRUE);
}
gboolean process_time_sync(MonkeyClient *mc) {
	GTimeVal local_time;	
	MonkeyMessage *m = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
	
	if (!read_update_from_monkey_server(m,mc))
		return(FALSE);		
	g_get_current_time(&local_time);

	m->message = SYNC;

	m->time_stamp = network_game_get_time(PRIVATE(mc)->network_game);

#ifdef _TIME_DEBUG_
	g_print("**DEBUG** MoneyClient->process_time_sync() : sending time ref %d\n", m->time_stamp);
#endif			

	if (!monkey_client_send_update_to_monkey_server(m,mc))
		return(FALSE);
	if (!read_update_from_monkey_server(m,mc))
		return(FALSE);								
	if (m->message == ACK) {
#ifdef _TIME_DEBUG_
	g_print("**DEBUG** MoneyClient->process_time_sync() : got time\n");
#endif						
			network_game_time_init(PRIVATE(mc)->network_game, &local_time);	
			return(TRUE);			
	}
	return(FALSE);
}

gboolean monkey_client_send_update_to_monkey_server(MonkeyMessage *m, MonkeyClient *mc) {

#ifdef _DEBUG_   
   fprintf(stdout,"**DEBUG** MonkeyClient->monkey_client_send_update_to_monkey_server()  : Sending...\n");
#endif	
	if (write(PRIVATE(mc)->socket, m, sizeof(struct _monkeyMessage)) < 1)
	{
		perror("write()");
#ifdef _DEBUG_
	fprintf(stdout,"***DEBUG*** MonkeyClient->monkey_client_send_update_to_monkey_server() : got socket full of moisture\n");
#endif	      		
		return(FALSE);		
	}
#ifdef _DEBUG_   
   fprintf(stdout,"**DEBUG** MonkeyClient->monkey_client_send_update_to_monkey_server()  : [OK]\n");   
#endif   
	return(TRUE);
}

gboolean read_update_from_monkey_server(MonkeyMessage *m, MonkeyClient *mc) {

#ifdef _DEBUG_   
   fprintf(stdout,"**DEBUG**  MonkeyClient->read_update_from_monkey_server() : Receiving...\n");
#endif  	
   if (read(PRIVATE(mc)->socket, m, sizeof(struct _monkeyMessage)) < 1)
   {
      perror("read()");
#ifdef _DEBUG_
	fprintf(stdout,"***DEBUG*** MonkeyClient->read_update_from_monkey_server() : got socket full of moisture\n");
#endif	            
      return(FALSE);
   }
	return(TRUE);
}
GHashTable *monkey_client_get_other_monkey_players(MonkeyClient *mc) {
	return(PRIVATE(mc)->foreign_monkeys);
}

