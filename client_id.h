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
	char *id;
	int socket;
	int active;
} provider_id_t;

typedef struct reader_id_t {
	int id;
	char *topic;
	int socket;
	int active;
} reader_id_t;

provider_id_t* makeProviderID(buffer_t *buffer, pthread_t worker, char *id, int socket);

reader_id_t* makeReaderID(int id, char *topic, int socket);

#endif /* end of include guard: CLIENT_ID_H */
