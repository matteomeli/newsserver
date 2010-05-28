/*
 *  client_id.c
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "client_id.h"

provider_id_t* makeProviderID(buffer_t *buffer, pthread_t worker, 
															char *name, char *topic, 
															int id, int socket) {
	provider_id_t *provider_id = (provider_id_t *)malloc(sizeof(provider_id_t));
	provider_id->name = (char *)malloc(strlen(name)+1);
	provider_id->topic = (char *)malloc(strlen(topic)+1);
	strcpy(provider_id->name, name);
	strcpy(provider_id->topic, topic);
	provider_id->buffer = buffer;
	provider_id->id = id;
	provider_id->worker = worker;
	provider_id->socket = socket;
	provider_id->active = 1;
	
	return provider_id;
}

reader_id_t* makeReaderID(pthread_t worker, 
													char *name, char *topic, 
													int id, int socket) {
	reader_id_t *reader_id = (reader_id_t *)malloc(sizeof(reader_id_t));
	reader_id->name = (char *)malloc(strlen(name)+1);
	reader_id->topic = (char *)malloc(strlen(topic)+1);
	strcpy(reader_id->topic, topic);
	strcpy(reader_id->name, name);
	reader_id->id = id;
	reader_id->worker = worker;
	reader_id->socket = socket;
	reader_id->active = 1;
	
	return reader_id;	
}

void destroyProviderID(provider_id_t *provider_id) {
	close(provider_id->socket);
	free(provider_id->name);
	free(provider_id->topic);
	free(provider_id);
}

void destroyReaderID(reader_id_t *reader_id) {
	close(reader_id->socket);
	free(reader_id->name);
	free(reader_id->topic);
	free(reader_id);
}