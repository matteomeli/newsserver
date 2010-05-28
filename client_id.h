/*
 *  client_id.h
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef CLIENT_ID_H
#define CLIENT_ID_H

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "buffer.h"

typedef struct provider_id_t {
	buffer_t *buffer;
	pthread_t worker;
	char *name;
	char *topic;
	int id;
	int socket;
	int active;
} provider_id_t;

typedef struct reader_id_t {
	pthread_t worker;
	char *name;
	char *topic;
	int id;
	int socket;
	int active;
} reader_id_t;

provider_id_t* makeProviderID(buffer_t *buffer, pthread_t worker, 
															char *name, char *topic, 
															int id, int socket);

reader_id_t* makeReaderID(pthread_t worker, char *name, char *topic, 
													int id, int socket);

void destroyProviderID(provider_id_t *provider_id);

void destroyReaderID(reader_id_t *reader_id);

#endif /* end of include guard: CLIENT_ID_H */
