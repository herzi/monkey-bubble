/* main.c - 
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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

static MonkeyServer *srv;
void interrupt_catch(int);
static gint only_master;

// FIX by lolo, respect the code law
// can it be a static method
void interrupt_catch(int s)
{    
	if (only_master == 0) {
		only_master++;
	    g_print("CATCH SIGINT...\nExiting\n"); 
	    g_object_unref(srv);        
	    exit(-1);
	}
}
int main(int argc, char **argv) {
	GError **err = NULL;
	GThread *thr = NULL;
	
    signal(SIGINT, interrupt_catch);

    g_type_init();
	if ((srv = (MonkeyServer *) monkey_server_new()) == NULL) {
    	fprintf(stdout, "Unable to create server\n");	
    	exit(-1);
    }		   
    if (!monkey_server_init(srv,(unsigned short) MONKEY_PORT)) {
    	fprintf(stdout, "Unable to initialize server\n");	
    	exit(-1);
    }    
	thr = g_thread_create((GThreadFunc) monkey_server_start, srv, TRUE, err);
	g_thread_join(thr); 
	return(0);
}
