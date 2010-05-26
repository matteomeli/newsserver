/*
 *  reader.h
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef READER_H
#define READER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

#include "list.h"

#define NEWS_ADDRESS				"127.0.0.1"
#define NEWS_SUBSCRIBE			8889
#define SIZE_BUFFER					64
#define QUIT								-4

typedef struct reader_t {
	int id;
	char *name;
	char *topic;
	int socket;
	list_t *received;
} reader_t;

reader_t* initReader(int argc, const char *argv[]);

int connectToServer(reader_t *reader);

int subscribeReader(reader_t *reader);

int getNews(reader_t *reader);

int runReader(reader_t *reader);

void destroyReader(reader_t *reader);

#endif /* end of include guard: READER_H */