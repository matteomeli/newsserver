/*
 *  provider.h
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef PROVIDER_H
#define PROVIDER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>

#include "pc.h"

#define NEWS_ADDRESS			"127.0.0.1"
#define NEWS_PUBLISH			8888
#define SIZE_BUFFER				64
#define NAME_KO						-1

typedef struct provider_t {
	int 	id;
	char 	*name;
	char 	*topic;
	int 	socket;
	int 	msgs;
	int 	delay;
	int 	live;
} provider_t;

provider_t* initProvider(int argc, const char *argv[]);

int connectToServer(provider_t *provider);

int advertiseProvider(provider_t *provider);

int sendNews(provider_t *provider);

int runProvider(provider_t *provider);

void destroyProvider(provider_t *provider);

#endif /* end of include guard: PROVIDER_H */