/* monkey-server.c - 
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
 
 #include "monkey-server.h"
#include "client-handler.h"
#include "clock.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <glib/gthread.h>
#include <sys/time.h>
#include <time.h>

struct MonkeyServerPrivate {
	int socket;
	unsigned short port;
	gboolean is_running;
//	GTimeVal main_time;	
	Clock *clock;
	GSList *connected_clients;
	gint game_state;
	unsigned long game_offset;	
};

#define PRIVATE( MonkeyServer ) (MonkeyServer->private)

static GObjectClass* parent_class = NULL;
static GStaticMutex network_update_mutex = G_STATIC_MUTEX_INIT;
static GStaticMutex model_update_mutex = G_STATIC_MUTEX_INIT;
static GStaticMutex update_game_state_mutex = G_STATIC_MUTEX_INIT;

int accept_client(MonkeyServer *);
gint compare_clients(gconstpointer , gconstpointer);
void monkey_server_finalize(GObject *);
static void monkey_server_iclient_handler_observer_iface_init(IClientHandlerObserverClass *);
static void monkey_server_propagate_model_update(IClientHandlerObserver *, ClientHandler *, MonkeyMessage *);


	
MonkeyServer *monkey_server_new() {
	MonkeyServer *s;
	
#ifdef G_THREADS_ENABLED  
	if (!g_thread_supported()) 
		g_thread_init (NULL);
#endif
	s = MONKEY_SERVER(g_object_new(TYPE_MONKEY_SERVER, NULL));
	
	g_assert(IS_MONKEY_SERVER(s));	
	PRIVATE(s)->game_state = GAME_NOT_READY;
	PRIVATE(s)->clock = clock_new();
	clock_start(PRIVATE(s)->clock);
	PRIVATE(s)->socket = -1;
	PRIVATE(s)->port = -1;
	PRIVATE(s)->is_running = TRUE;
	return(s);

}
void monkey_server_finalize(GObject *object) {
	int i;
	MonkeyServer *srv = (MonkeyServer *) object;
	
	PRIVATE(srv)->is_running = FALSE;
	close(PRIVATE(srv)->socket);
	for (i=0; i < (g_slist_length(srv->private->connected_clients));i++) {
		client_handler_detach_observer((g_slist_nth(PRIVATE(srv)->connected_clients, i))->data,ICLIENT_HANDLER_OBSERVER(srv));
		if (((g_slist_nth(PRIVATE(srv)->connected_clients, i))->data) != NULL)
			g_object_unref((g_slist_nth(PRIVATE(srv)->connected_clients, i))->data);
	}
	g_slist_free(PRIVATE(srv)->connected_clients);
	g_free(PRIVATE(srv));

	 if (G_OBJECT_CLASS (parent_class)->finalize) {
    	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
  	}		
}
gboolean monkey_server_init(MonkeyServer *srv,unsigned short port) 
{
	struct sockaddr_in sock_client;
  	
	if ((PRIVATE(srv)->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
      perror("socket() ");
		return(FALSE);
	}			
	bzero((char *) &sock_client, sizeof(sock_client));
	sock_client.sin_family = AF_INET;
	sock_client.sin_addr.s_addr = INADDR_ANY;
	sock_client.sin_port = htons(port);	
	if (bind(PRIVATE(srv)->socket, (struct sockaddr *) &sock_client, sizeof(sock_client)) == -1)
	{
		perror("bind() ");
		return(FALSE);
	}
	if (listen(PRIVATE(srv)->socket, 4) == -1)
   {
      perror("listen() ");
      return(FALSE);
   }	
   if (port != 0)
		return(TRUE);
	else 
	{
		int toto = sizeof(sock_client);
		if (getsockname(PRIVATE(srv)->socket,(struct sockaddr *) &sock_client, &toto) == -1)
		{
			perror("getsockname() ");
			return(FALSE);
		}
	}
	PRIVATE(srv)->port = ntohs(sock_client.sin_port);
	return(TRUE);			
}
static void monkey_server_instance_init(MonkeyServer * s) {
  s->private =g_new0 (MonkeyServerPrivate, 1);			
}
static void monkey_server_class_init (MonkeyServerClass *klass) {
    GObjectClass* object_class;
    
    parent_class = g_type_class_peek_parent(klass);
    object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = monkey_server_finalize;
}
GType monkey_server_get_type(void) {
    static GType monkey_server_type = 0;
    
    if (!monkey_server_type) {
      static const GTypeInfo monkey_server_info = {
	sizeof(MonkeyServerClass),
	NULL,           /* base_init */
	NULL,           /* base_finalize */
	(GClassInitFunc) monkey_server_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof(MonkeyServer),
	1,              /* n_preallocs */
	(GInstanceInitFunc) monkey_server_instance_init,
      };
      
   static const GInterfaceInfo iface_iclient_handler_observer = {
	(GInterfaceInitFunc) monkey_server_iclient_handler_observer_iface_init,
	NULL,
	NULL
      };
      monkey_server_type = g_type_register_static(G_TYPE_OBJECT,
						"MonkeyServer",
						&monkey_server_info, 0
						);
      g_type_add_interface_static(monkey_server_type,
				  TYPE_ICLIENT_HANDLER_OBSERVER,
				  &iface_iclient_handler_observer);      						

    }
    
    return monkey_server_type;
}

void *monkey_server_start(MonkeyServer *srv ) {
	while (PRIVATE(srv)->is_running)
	{		
		ClientHandler *ch;		
		GError **err = NULL;
		GThread *thr = NULL;
		
#ifdef _DEBUG_
			fprintf(stdout,"**DEBUG** accepting clients incoming connections....\n");
#endif			

		ch  = client_handler_new(accept_client(srv), srv);
		if (g_slist_length(PRIVATE(srv)->connected_clients) == 0)
			client_handler_set_game_creator(ch);
			
		PRIVATE(srv)->connected_clients = g_slist_append(PRIVATE(srv)->connected_clients, ch);
		thr = g_thread_create((GThreadFunc) client_handler_start, ch, TRUE, err);
		client_handler_set_thread(ch, thr);
		client_handler_attach_observer(ch,ICLIENT_HANDLER_OBSERVER(srv));
								
#ifdef _DEBUG_		
		fprintf(stdout,"**DEBUG** Done!!....\n");
#endif
	}
	g_thread_exit(0);	
	return(0);
}
gint compare_clients(gconstpointer a, gconstpointer b) {
	ClientHandler *ch1 = (ClientHandler *) a;
	ClientHandler *ch2 = (ClientHandler *) b;		

	if (client_handler_get_id(ch1) == client_handler_get_id(ch2))
		return(0);
	return(-1);
	
}
void monkey_server_kill_client(ClientHandler *ch, MonkeyServer *srv) {
	GSList *lst;

	if ((lst = g_slist_find_custom(srv->private->connected_clients, ch,compare_clients)) == NULL)
		fprintf(stderr, "MonkeyServer->monkey_server_kill_client() : Unknown Client (%d)\n",client_handler_get_id(ch));
	else {				
		client_handler_detach_observer(ch,ICLIENT_HANDLER_OBSERVER(srv));			
		PRIVATE(srv)->connected_clients = g_slist_remove(PRIVATE(srv)->connected_clients,lst->data);		
		if (lst->data != NULL)	
			g_object_unref(lst->data);			
	}	
}

int accept_client(MonkeyServer *srv) {

	struct sockaddr_in sock_client;
	int sock;
	int lg_info = sizeof(sock_client);
				
	if ((sock = accept(PRIVATE(srv)->socket, (struct sockaddr *) &sock_client, &lg_info)) == -1) {	
		perror("accept() ");
		close(sock);
		g_object_unref(srv);
		exit(-1);
	}
#ifdef _DEBUG_
	fprintf(stdout,"**DEBUG** MonkeyServer->accept_client() : Connexion Accepted...., returning fd : %d\n",sock);
#endif    
	fprintf(stdout,"Got Connexion from %s\n", (char *) inet_ntoa(sock_client.sin_addr));
	return(sock);
}

gboolean monkey_server_propagate_network_update(MonkeyMessage *m, MonkeyServer *srv) {    
	int i;
//propagate to all client's side except client side which send event
    g_static_mutex_lock (&network_update_mutex);
#ifdef _DEBUG_
	fprintf(stdout,"**DEBUG** : inside critical section\n");
	fprintf(stdout,"**DEBUG** : %d Connected Clients\n",g_slist_length(srv->private->connected_clients));
#endif
	for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++)
	{				
		GSList *elt = g_slist_nth(PRIVATE(srv)->connected_clients,i);
		if (elt == NULL) {
			fprintf(stderr, "MonkeyServer->monkey_server_propagate_network_update() : Can't retrieve ClientHandler\n");
			g_static_mutex_unlock(&network_update_mutex);	
			return (FALSE);
		}
		else {			
#ifdef _DEBUG_
			ClientHandler *ch = (ClientHandler *) elt->data;		
			fprintf(stdout,"**DEBUG** monkey_server_propagate_network_update() : Got GSList, handler is %d, message is %d at %d\n", client_handler_get_id(ch),m->message, m->time_stamp);
#endif	
			if (m->from != client_handler_get_id(elt->data))
				if (!send_update_to_client(m, elt->data)) {
						fprintf(stderr,"monkey_server_propagate_network_update() : Unable to transmit update to client, Killing handler.\n");
						g_object_unref(elt->data);            
						g_static_mutex_unlock(&network_update_mutex);																	
						return(FALSE);																	
			}
		}
	}
		g_static_mutex_unlock(&network_update_mutex);							
#ifdef _DEBUG_
		fprintf(stdout,"**DEBUG** : end of critical section\n");
#endif				
		return(TRUE);		
}
static void monkey_server_propagate_model_update(IClientHandlerObserver *cho, ClientHandler * ch, MonkeyMessage *m) {
	gint i;
	gint gaming_players = 0;
	MonkeyServer *srv = (MonkeyServer *) cho;
//propagate to all client's side	
	g_static_mutex_lock (&model_update_mutex);
#ifdef _DEBUG_
	fprintf(stdout,"**DEBUG** : inside critical section\n");
	fprintf(stdout,"**DEBUG** : %d Connected Clients\n",g_slist_length(srv->private->connected_clients));
#endif
	for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++) {				
		GSList *elt = g_slist_nth(PRIVATE(srv)->connected_clients,i);
		if (elt == NULL) {
			fprintf(stderr, "Can't retrieve ClientHandler\n");
			return ;
		}
		else {			
#ifdef _DEBUG_
			ClientHandler *ch = (ClientHandler *) elt->data;		
			fprintf(stdout,"**DEBUG**monkey_server_propagate_model_update() : Got GSList, handler is %d, message is %d\n", client_handler_get_id(ch),m->message);
#endif				
			if (!send_update_to_client(m, elt->data)) {
					fprintf(stderr,"monkey_server_propagate_model_update() : Unable to transmit update to client, Killing handler.\n");
					g_object_unref(elt->data);            
					g_static_mutex_unlock(&model_update_mutex);																	
					return;																	
			}
			if (client_handler_get_state(ch) != PLAYER_GAMING)
				gaming_players++;
		}
	}	
			
	if (gaming_players == 1) {
		//belle tete de vainqueur
		m->message = PLAYER_WIN;
		//sending to all client side player_win to players
		for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++) {				
			GSList *elt = g_slist_nth(PRIVATE(srv)->connected_clients,i);
			if (elt == NULL) {
				fprintf(stderr, "Can't retrieve ClientHandler\n");
				return;
			}
			else {			
#ifdef _DEBUG_
				ClientHandler *ch = (ClientHandler *) elt->data;		
				fprintf(stdout,"**DEBUG**monkey_server_propagate_model_update() : Got GSList, handler is %d, message is %d\n", client_handler_get_id(ch),m->message);
#endif				
				if (!send_update_to_client(m, elt->data)) {
						fprintf(stderr,"monkey_server_propagate_model_update() : Unable to transmit update to client, Killing handler.\n");
						g_object_unref(elt->data);            
						g_static_mutex_unlock(&model_update_mutex);																	
						return;																	
				}
			}
		}
		//updating game state
		PRIVATE(srv)->game_state = GAME_FINISHED;
		//something else to do?		
	}		
	g_static_mutex_unlock(&model_update_mutex);		
#ifdef _DEBUG_
	fprintf(stdout,"**DEBUG** : end of critical section\n");
#endif										
}
static void monkey_server_iclient_handler_observer_iface_init(IClientHandlerObserverClass * i) {
  i->event = monkey_server_propagate_model_update;
}
GSList *monkey_server_get_monkeys(MonkeyServer *srv){
	GSList *monkeys = NULL;
	int i;
	
	for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++) {

		GSList *lst = g_slist_nth(PRIVATE(srv)->connected_clients, i);
		monkeys = g_slist_append(monkeys, GINT_TO_POINTER(client_handler_get_id(lst->data)));
	}
	return (monkeys);	
}
gint monkey_server_get_game_state(MonkeyServer *srv) {
	return(PRIVATE(srv)->game_state);
}
gboolean monkey_server_start_game(MonkeyServer *srv,MonkeyMessage *m) {
		if (PRIVATE(srv)->game_state == GAME_READY) {			
			MonkeyMessage *mm = (MonkeyMessage *) g_malloc(sizeof( struct _monkeyMessage));
			PRIVATE(srv)->game_state = GAME_STARTED;
			mm->message = PROCESS_SYNCHRONIZATION;
			if (monkey_server_propagate_network_update(mm, srv)) {
				g_free(mm);
				return(TRUE);
			}
			g_free(mm);
		}
		return(FALSE);		
}
void monkey_server_update_game_state(MonkeyServer *srv, MonkeyMessage *m) {
	int i;
	MonkeyMessage *mm;
	g_static_mutex_lock (&update_game_state_mutex);
	switch (m->message) {
		case PLAYER_READY:
		case PLAYER_NOT_READY:			for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++) {
																	ClientHandler *ch =(ClientHandler *) (g_slist_nth(PRIVATE(srv)->connected_clients, i))->data;
																	if (client_handler_get_state(ch) == PLAYER_NOT_READY) {
																		g_print("Player not ready!\n");
																		g_static_mutex_unlock (&update_game_state_mutex);
																		return;
																	}
																}
																PRIVATE(srv)->game_state = GAME_READY;	
																g_print("**DEBUG*** Game in ready mode\n");
																break;
		case PLAYER_SYNCHRONIZED:				for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++) {
																	ClientHandler *ch =(ClientHandler *) (g_slist_nth(PRIVATE(srv)->connected_clients, i))->data;
																	if (client_handler_get_state(ch) != PLAYER_SYNCHRONIZED) {
#ifdef _DEBUG_
																		g_print("**DEBUG** MonkeyServer->update_game_state : found not synchronized player (%d)\n",client_handler_get_id(ch));
#endif
																		g_static_mutex_unlock (&update_game_state_mutex); 																	
																		return;
																	}
																}
#ifdef _DEBUG_
																g_print("**DEBUG** MonkeyServer->update_game_state : All Players Synchronized\n");
#endif															
																mm = (MonkeyMessage *) g_malloc(sizeof(struct _monkeyMessage));
																mm->message = BEGIN_GAME;
																mm->time_stamp = monkey_server_get_time(srv);
#ifdef _DEBUG_
																g_print("**DEBUG** MonkeyServer->update_game_state : Giving Game start time %d\n", mm->time_stamp);
#endif																		
																monkey_server_propagate_network_update(mm, srv);
																PRIVATE(srv)->game_state = GAME_PLAYING;
																PRIVATE(srv)->game_offset = mm->time_stamp;																
																for (i=0; i < (g_slist_length(PRIVATE(srv)->connected_clients));i++) {
																	ClientHandler *ch =(ClientHandler *) (g_slist_nth(PRIVATE(srv)->connected_clients, i))->data;
																	client_handler_start_game(ch);
																}
																g_free(mm);																	
																break;
			case PLAYER_LOST:				
															break;
			default: g_print("Don't know what to do with game state %d\n", m->message);
							break;
	}
	g_static_mutex_unlock (&update_game_state_mutex);	
}
gint monkey_server_get_time(MonkeyServer * srv) {
  return clock_get_time(PRIVATE(srv)->clock);
}

unsigned long monkey_server_get_game_offset(MonkeyServer *srv) {return PRIVATE(srv)->game_offset;}
