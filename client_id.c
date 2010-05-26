/*
 *  client_id.c
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "client_id.h"

provider_id_t* makeProviderID(buffer_t *buffer, pthread_t worker, char *id, int socket) {
	provider_id_t *provider_id = (provider_id_t *)malloc(sizeof(provider_id_t));
	provider_id->buffer = buffer;
	provider_id->worker = worker;
	provider_id->id = (char *)malloc(strlen(id)+1);
	strcpy(provider_id->id, id);
	provider_id->socket = socket;
	provider_id->active = 1;
	
	return provider_id;
}

reader_id_t* makeReaderID(int id, char *topic, int socket) {
	reader_id_t *reader_id = (reader_id_t *)malloc(sizeof(reader_id_t));
	//reader_id->id = (char *)malloc(strlen(id)+1);
	//strcpy(reader_id->id, id);
	reader_id->id = id;
	reader_id->topic = (char *)malloc(strlen(topic)+1);
	strcpy(reader_id->topic, topic);
	reader_id->socket = socket;
	reader_id->active = 1;
	
	return reader_id;	
}