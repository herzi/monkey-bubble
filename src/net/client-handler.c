/* client-handler.c - 
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
 
#include "client-handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <gtk/gtk.h>

#define PRIVATE( ClientHandler ) (ClientHandler->private)


gboolean init_game(ClientHandler *);
gboolean client_handler_process_message(ClientHandler *, MonkeyMessage *);
gboolean read_update_from_client(MonkeyMessage *, ClientHandler *);
gboolean process_sync(ClientHandler *);
void client_handler_finalize(GObject *);
void update_local_monkey(ClientHandler *,MonkeyMessage *);
static void client_handler_imonkey_observer_iface_init(IMonkeyObserverClass * i);
static void client_handler_game_lost(IMonkeyObserver *bo,Monkey * monkey);
static void client_handler_bubbles_exploded(IMonkeyObserver * bo, Monkey * monkey, GList * exploded, GList * fallen);
static void client_handler_bubble_shot(IMonkeyObserver * bo, Monkey * monkey, Bubble * bubble);
static void client_handler_bubble_sticked(IMonkeyObserver * bo,  Monkey * monkey,  Bubble * bubble);

static void client_handler_bubbles_waiting_changed(IMonkeyObserver * bo,
																	Monkey * monkey,
																	int bubbles_count);
static gint monkey_timeout(gpointer);

static GObjectClass* parent_class = NULL;

struct ClientHandlerPrivate {
	int socket;
	GThread *thr;
	gboolean is_running;
	MonkeyServer *srv;	
	unsigned long network_offset;	
	Monkey *monkey;
	GList * observerList;
	unsigned long id;
	gint player_state;
	gboolean game_creator;
	guint timeout;		
};


ClientHandler *client_handler_new(int socket, MonkeyServer * srv)
{
	ClientHandler *ch;
	ch = CLIENT_HANDLER(g_object_new(TYPE_CLIENT_HANDLER, NULL));
	
	g_assert(IS_CLIENT_HANDLER(ch));	
	
	PRIVATE(ch)->monkey = monkey_new();
	PRIVATE(ch)->socket = socket;
	PRIVATE(ch)->thr = NULL;
	PRIVATE(ch)->srv = srv;
	PRIVATE(ch)->is_running = TRUE;	
	PRIVATE(ch)->id = socket;	
	PRIVATE(ch)->player_state = PLAYER_NOT_READY;	
	PRIVATE(ch)->game_creator = FALSE;

	monkey_attach_observer(PRIVATE(ch)->monkey, IMONKEY_OBSERVER(ch));
	return(ch);
}
void client_handler_finalize(GObject *object) {
	MonkeyMessage *m;
	ClientHandler *ch = (ClientHandler *) object;

	if(PRIVATE(ch)->observerList != NULL) {
    	g_error("[ClientHandler] All observer has not been removed");		
  	}
	m = (struct _monkeyMessage *) g_malloc (sizeof(struct _monkeyMessage));	
	m->message = CLIENT_MUST_QUIT;

	send_update_to_client(m,ch);
	PRIVATE(ch)->is_running = FALSE;
	PRIVATE(ch)->id = -1;
	close(PRIVATE(ch)->socket);
    monkey_detach_observer(PRIVATE(ch)->monkey,  IMONKEY_OBSERVER(ch));
	if ((PRIVATE(ch)->monkey) != NULL)
		g_object_unref(PRIVATE(ch)->monkey);
	g_free(m);
	g_free(PRIVATE(ch));		

	 if (G_OBJECT_CLASS (parent_class)->finalize) {
    	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
  	}	

}
static void client_handler_instance_init(ClientHandler * ch) {
  PRIVATE(ch) =g_new0 (ClientHandlerPrivate, 1);			
}
static void client_handler_class_init (ClientHandlerClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = client_handler_finalize;
}
GType client_handler_get_type(void) {
    static GType client_handler_type = 0;
    
    if (!client_handler_type) {
      static const GTypeInfo client_handler_info = {
	sizeof(ClientHandlerClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) client_handler_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(ClientHandler),
	1,              /* n_preallocs */
	(GInstanceInitFunc) client_handler_instance_init,
      };

      static const GInterfaceInfo iface_imonkey_observer = {
	(GInterfaceInitFunc) client_handler_imonkey_observer_iface_init,
	NULL,
	NULL
      };
      		
      
      client_handler_type = g_type_register_static(G_TYPE_OBJECT,
						"ClientHandler",
						&client_handler_info, 0);
      
      g_type_add_interface_static(client_handler_type,
				  TYPE_IMONKEY_OBSERVER,
				  &iface_imonkey_observer);		
    }
    
    return client_handler_type;
}

void client_handler_set_thread(ClientHandler *ch, GThread *thr) {
	PRIVATE(ch)->thr = thr;
}
void *client_handler_start(ClientHandler *ch) {
	MonkeyMessage *m;
	m = (struct _monkeyMessage *) g_malloc (sizeof(struct _monkeyMessage));		
		
	while (PRIVATE(ch)->is_running) {
		if (!read_update_from_client(m,ch)) {
			fprintf(stderr, "ClientHandler->start : Unable to communicate with client %ld, Handler exiting ...\n", PRIVATE(ch)->id);
			monkey_server_kill_client(ch, ch->private->srv);
		}
		else {					
			if (!client_handler_process_message(ch, m))
				fprintf(stderr,"Unable to process message\n");		
		}
	}
	g_free(m);
	g_thread_exit(0);	
	return(0);
}
gboolean client_handler_process_message(ClientHandler *ch, MonkeyMessage *m) {
	MonkeyMessage *mm = (struct _monkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));

	 
#ifdef _TIME_DEBUG_
	g_print("**DEBUG** ClientHandler->process_message : got message at %ld with offset %ld, so time is %d\n",m->time_stamp+PRIVATE(ch)->network_offset, PRIVATE(ch)->network_offset,	(gint)m->time_stamp);
#endif	
	//label message
	m->from = PRIVATE(ch)->id;
	switch(m->message) {
		case NACK: 									g_print( "Got Error Message\n");
					  											break;
		case ACK: 
#ifdef _DEBUG_
																g_print("**DEBUG** Got ACK\n");
#endif
					  											break; 
		case SYNC: 
#ifdef _DEBUG_
																g_print("**DEBUG** Got SYNC\n");
#endif
																if (monkey_server_get_game_state(PRIVATE(ch)->srv) == GAME_STARTED) {
																	if (!init_game(ch))
																		g_print("Failed to initialize client game\n");												
																}
																else g_print("Got SYNC when game is not in ready state!!! (game is %d)\n",monkey_server_get_game_state(PRIVATE(ch)->srv) );
					 	   										break;					 	  				 
		case START_GAME:
#ifdef _DEBUG_
																g_print("**DEBUG** Got START GAME\n");
#endif
																if (PRIVATE(ch)->game_creator) {
																	if (!monkey_server_start_game(PRIVATE(ch)->srv, m)) {
																			g_print("Unable to initiate game creation process, client exiting ...\n");
																			monkey_server_kill_client(ch, PRIVATE(ch)->srv);
																	}
																}
																else g_print("Got START GAME from a player another player than game creator, ignoring ...\n");
																break;																						
		case SHOOT:  		
		case MOVE_LEFT_PRESSED:
		case MOVE_LEFT_RELEASED:
		case MOVE_RIGHT_RELEASED: 
		case MOVE_RIGHT_PRESSED: 
#ifdef _DEBUG_
																g_print("**DEBUG** Got SHOOT|MOVE Event (%d) at %d\n", m->message, (gint) m->time_stamp);
#endif
																monkey_server_propagate_network_update(m,PRIVATE(ch)->srv);
																update_local_monkey(ch,m);													 			
																break;				
		case PLAYER_WANTS_TO_QUIT: 		mm->message = CLIENT_MUST_QUIT;																
																send_update_to_client(mm,ch);
														    	break;
		case PLAYER_READY:						
#ifdef _DEBUG_
																g_print("**DEBUG** Got PLAYER_READY at %d\n",  (gint) m->time_stamp);
#endif														
																PRIVATE(ch)->player_state = PLAYER_READY;
																monkey_server_update_game_state(PRIVATE(ch)->srv, m);
																break;
		case PLAYER_NOT_READY:				
#ifdef _DEBUG_
																g_print("**DEBUG** Got PLAYER_NOT_READY at %d\n",  (gint) m->time_stamp);
#endif														
																PRIVATE(ch)->player_state = PLAYER_NOT_READY;
																monkey_server_update_game_state(PRIVATE(ch)->srv, m);		
																break;
		case PLAYER_WANTS_PAUSE:			//todo
																break;
																										
		default: 											g_print("ClientHandler->process_message : Got Unknown Message %d\n", m->message);
					 											break;
						 
	}
	g_free(mm);
	return(TRUE);		
}
void update_local_monkey(ClientHandler *ch,MonkeyMessage *m) {

	m->time_stamp -= monkey_server_get_game_offset(PRIVATE(ch)->srv);
#ifdef _DEBUG_
		g_print("**DEBUG** : ClientHandler->update_local_monkey() : Updating local monkey message %d at %d\n", m->message, m->time_stamp);
#endif
		switch(m->message) {
		case SHOOT: monkey_shoot(PRIVATE(ch)->monkey,m->time_stamp);
							 break;
		case MOVE_LEFT_PRESSED: 	monkey_left_changed(PRIVATE(ch)->monkey, TRUE, m->time_stamp);
														break;
		case MOVE_LEFT_RELEASED: 	monkey_left_changed(PRIVATE(ch)->monkey, FALSE, m->time_stamp);
														break;
		case MOVE_RIGHT_PRESSED: 	monkey_right_changed(PRIVATE(ch)->monkey, TRUE, m->time_stamp);
														break;
		case MOVE_RIGHT_RELEASED: monkey_right_changed(PRIVATE(ch)->monkey, FALSE, m->time_stamp);
														break;				
	}
	monkey_update(PRIVATE(ch)->monkey,m->time_stamp);	
}
gboolean init_game(ClientHandler *ch) {
		
	MonkeyMessage *m;
	GSList *clts = NULL;
	int i;

	m = (struct _monkeyMessage *) g_malloc (sizeof(struct _monkeyMessage));	

	//send clientHandler id
#ifdef _DEBUG_
	g_print("***DEBUG*** : ClientHandler->init_game() : Sending id\n");
#endif	
	m->message = CLIENT_ID;
	m->arg.id = htonl(PRIVATE(ch)->id);
	send_update_to_client( m, ch);
#ifdef _DEBUG_
	g_print("***DEBUG*** : ClientHandler->init_game() : Sending Player List\n");
#endif	
	//sending players list
	m->message = PLAYER_LIST;
	clts = monkey_server_get_monkeys(PRIVATE(ch)->srv);
#ifdef _DEBUG_
	g_print("***DEBUG*** : ClientHandler->init_game() : Got ids, building message\n");
#endif	
	m->arg.players[0] =  htonl((gint) g_slist_length(clts));		
	for (i=0; i < (g_slist_length(clts));i++) 
		m->arg.players[i+1] =  htonl((gint) (g_slist_nth(clts, i))->data);

	if (!send_update_to_client(m,ch)) {
		g_print("Unable to send PLAYER_LIST to client\n");	
		return FALSE;
	}
#ifdef _DEBUG_
	g_print("***DEBUG*** : ClientHandler->init_game() : Processing Clock Syncronization\n");
#endif	
	//clock syncronization	
	process_sync(ch);

	//wait for ACK to update clienthandler state
	if (!read_update_from_client(m,ch)) {
			g_print("ClientHandler->init_game() : Unable to communicate with client, Handler exiting ...\n");
			monkey_server_kill_client(ch, PRIVATE(ch)->srv);
			return(FALSE);
	}
	if (m->message != ACK) {
		g_free(m);
		return(FALSE);
	}
	PRIVATE(ch)->player_state = PLAYER_SYNCHRONIZED;
	//temporaire, je me sers de message pour update (comprendre l integralite de la struct message n'est pas necessaire)
	m->message = PLAYER_SYNCHRONIZED;	
	monkey_server_update_game_state(PRIVATE(ch)->srv, m);
	
#ifdef _DEBUG_		
		g_print("***DEBUG***  : ClientHandler->init_game() : Sync done!\n");	
#endif
	g_free(m);	
	return(TRUE);
}

gboolean process_sync(ClientHandler *ch) {
	MonkeyMessage *m;
	m = (struct _monkeyMessage *) g_malloc (sizeof(struct _monkeyMessage));	
	//send clock ref
	m->message = SYNC;
	m->time_stamp = monkey_server_get_time(PRIVATE(ch)->srv);
	if (!send_update_to_client(m,ch)) {
		fprintf(stderr,"ClientHandler->process_sync() : Unable to send time sync to client\n");
		return(FALSE);
	}
	if (!read_update_from_client(m,ch)) {
			fprintf(stderr, "ClientHandler->process_sync() : Unable to communicate with client, Handler exiting ...\n");
			monkey_server_kill_client(ch, PRIVATE(ch)->srv);
			g_free(m);
			return(FALSE);
	}
	if (m->message != SYNC) {
		fprintf(stdout, "Not a SYNC EVENT!!");	
			g_free(m);		
		return(FALSE);
	}
	else {
		PRIVATE(ch)->network_offset = m->time_stamp - monkey_server_get_time(PRIVATE(ch)->srv);
#ifdef _TIME_DEBUG_
		fprintf(stdout,"***DEBUG*** : Got message %d\n",m->message);
		fprintf(stdout,"***DEBUG*** : Local time is %d\n",monkey_server_get_time(PRIVATE(ch)->srv));
		fprintf(stdout,"***DEBUG*** : Client time is %d\n",m->time_stamp);	
		fprintf(stdout,"***DEBUG*** : Calculated offset is %ld\n", ch->private->network_offset);		
#endif	
	}
	m->message = ACK;
	send_update_to_client(m,ch);
	g_free(m);
	return(TRUE);			
}
gboolean send_update_to_client(MonkeyMessage *m, ClientHandler *ch) {

	//message shoold be reused. To leave back network offset

	m->time_stamp += PRIVATE(ch)->network_offset;
#ifdef _DEBUG_   
   fprintf(stdout,"**DEBUG** ClientHandler->send_update_to_client() : Sending...\n");
#endif	
	if (write(PRIVATE(ch)->socket, m, sizeof(struct _monkeyMessage)) < 1)
	{
		perror("write()");
		monkey_server_kill_client(ch, ch->private->srv);		
#ifdef _DEBUG_
		fprintf(stdout,"***DEBUG*** ClientHandler->send_update_to_client(): got socket full of moisture\n");
#endif	
		m->time_stamp -=  PRIVATE(ch)->network_offset;     
		return(FALSE);		
	}
#ifdef _DEBUG_   
   fprintf(stdout,"**DEBUG** ClientHandler->send_update_to_client() : Send [OK]\n");
#endif	
	m->time_stamp -=  PRIVATE(ch)->network_offset; 
	return(TRUE);
}

gboolean read_update_from_client(MonkeyMessage *m, ClientHandler *ch) {

#ifdef _DEBUG_   
   fprintf(stdout,"**DEBUG**  ClientHandler->read_update_from_client() : Receiving...\n");
#endif  	
   if (read(PRIVATE(ch)->socket, m, sizeof(struct _monkeyMessage)) < 1)
   {
      perror("read()");
#ifdef _DEBUG_
	fprintf(stdout,"***DEBUG*** ClientHandler->read_update_from_client(): got socket full of moisture, ClientHandler exiting\n");
	PRIVATE(ch)->is_running = FALSE;
#endif	      
      return(FALSE);
   }
   m->time_stamp-= PRIVATE(ch)->network_offset	;
	return(TRUE);
}

Monkey *client_handler_get_monkey(ClientHandler *ch) {
	return(PRIVATE(ch)->monkey);
}

void client_handler_attach_observer(ClientHandler * ch,IClientHandlerObserver *cho) {
  g_assert(IS_CLIENT_HANDLER(ch));
  g_assert(IS_ICLIENT_HANDLER_OBSERVER(cho));

  PRIVATE(ch)->observerList = g_list_append(PRIVATE(ch)->observerList,cho);
}

void client_handler_detach_observer(ClientHandler * ch,IClientHandlerObserver *cho) {
  g_assert(IS_CLIENT_HANDLER(ch));
  g_assert(IS_ICLIENT_HANDLER_OBSERVER(cho));

  PRIVATE(ch)->observerList = g_list_remove(PRIVATE(ch)->observerList,cho);
}
gint client_handler_get_id(ClientHandler *ch) {
	return(PRIVATE(ch)->id);
}
gint client_handler_get_state(ClientHandler *ch) {
	return(PRIVATE(ch)->player_state);
}
static void client_handler_imonkey_observer_iface_init(IMonkeyObserverClass * i) {

 imonkey_observer_class_virtual_init(i,
				      client_handler_game_lost,
				      client_handler_bubbles_exploded,
				      client_handler_bubble_shot,
				      client_handler_bubble_sticked,
												 client_handler_bubbles_waiting_changed);
}
static void client_handler_bubble_sticked(IMonkeyObserver * bo,  Monkey * monkey,  Bubble * bubble){
	//TODO ?
}

static void client_handler_bubbles_waiting_changed(IMonkeyObserver * bo,
																	Monkey * monkey,
																	int bubbles_count) {
}

static void client_handler_bubbles_exploded(IMonkeyObserver * bo, Monkey * monkey, GList * exploded, GList * fallen){
	IClientHandlerObserver * cho;
	GList * next;	
	MonkeyMessage *m;
	ClientHandler *ch = (ClientHandler *) bo;
  
  	m = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
  	m->from = PRIVATE(ch)->id;
  	m->message = BUBBLE_FALLEN;
  	/* TODO the bubbles ... */
#ifdef _TIME_DEBUG_
	g_print("**DEBUG** ClientHandler->client_handler_bubble_exploded() : Got Explode Event from observed Monkey\n");  	 
#endif 

  	next = PRIVATE(ch)->observerList;
  	while( next != NULL ) {
    	cho = (IClientHandlerObserver * )next->data;
    	iclient_handler_observer_event(cho,ch, m);
    	next = g_list_next(next);
  	} 
  	g_free(m);
}
static void client_handler_game_lost(IMonkeyObserver *bo,Monkey * monkey) {
 	IClientHandlerObserver * cho; 
 	GList * next;	 
 	MonkeyMessage *m;	 
 	ClientHandler *ch = (ClientHandler *) bo; 
    m = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage)); 
   	m->from = PRIVATE(ch)->id; 
   	m->message = PLAYER_LOST; 

#ifdef _TIME_DEBUG_ 
 	g_print("**DEBUG**  ClientHandler->client_handler_game_lost() : Got LOST Event from observed Monkey\n");  	  
#endif 
      
   	next = PRIVATE(ch)->observerList; 
   	while( next != NULL ) { 
     	cho = (IClientHandlerObserver * )next->data; 
     	iclient_handler_observer_event(cho,ch,m); 
     	next = g_list_next(next); 
   	}  
   	g_free(m);  	 	
}
static void client_handler_bubble_shot(IMonkeyObserver * bo, Monkey * monkey, Bubble * bubble) {
 	IClientHandlerObserver * cho; 
 	GList * next;	 	
	Bubble * b = bubble_new(rand()%7,0,0);
	MonkeyMessage *m = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
#ifdef _DEBUG_	
	g_print("**DEBUG** ClientHandler->client_handler_bubble_shot: Got Shoot Event from observed Monkey, updating next bubble (color %d)\n", bubble_get_color(b));	
#endif	
	g_assert( IS_CLIENT_HANDLER(bo));
    shooter_add_bubble(monkey_get_shooter(monkey),b);
    m->message =  NEXT_BUBBLE_TO_SHOOT;
    m->from = PRIVATE(CLIENT_HANDLER(bo))->id;
    m->arg.color =  bubble_get_color(b);	   
    next = PRIVATE(CLIENT_HANDLER(bo))->observerList; 
   	while( next != NULL ) { 
     	cho = (IClientHandlerObserver * )next->data; 
     	iclient_handler_observer_event(cho,CLIENT_HANDLER(bo),m); 
     	next = g_list_next(next); 
   	}  
   	g_free(m);        
}

void client_handler_set_game_creator(ClientHandler *ch) {
		PRIVATE(ch)->game_creator = TRUE;
	
}


void client_handler_start_game(ClientHandler *ch) {
	PRIVATE(ch)->timeout = gtk_timeout_add (GAME_TIMEOUT, monkey_timeout, ch);


}

static gint monkey_timeout (gpointer data) {
		
	g_assert(IS_CLIENT_HANDLER(data));

#ifdef _DEBUG_	
	g_print("**DEBUG** ClientHandler->monkey_timeout: Got Timeout, Upating\n");
#endif	

	monkey_update(PRIVATE(CLIENT_HANDLER(data))->monkey, monkey_server_get_time(PRIVATE(CLIENT_HANDLER(data))->srv) - monkey_server_get_game_offset(PRIVATE(CLIENT_HANDLER(data))->srv));			
	PRIVATE(CLIENT_HANDLER(data))->timeout = gtk_timeout_add (GAME_TIMEOUT, monkey_timeout, CLIENT_HANDLER(data));

	return(0);		
}

