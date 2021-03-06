/*
 *  server.h
 * 	Defines a news server interface.
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

// #include "accepter.h"
// #include "dispatcher.h"
#include "buffer.h"
#include "list.h"

#define NEWS_ADDRESS			"127.0.0.1"
#define NEWS_PUBLISH			8888
#define NEWS_SUBSCRIBE		8889
#define BACKLOG_SIZE			5

#define SIZE_NEWS					16
#define SIZE_BUFFER				64

#define NAME_KO						-1
#define SUCCESS						(void *)0
#define FAILURE						(void *)-1

typedef struct server_t {
	// accepter_t *accepter;
	// dispatcher_t *dispatcher;
	buffer_t *news;
	list_t *readers;
	list_t *providers;
	int socket_providers;
	int socket_readers;
} server_t;

server_t* initServer(int argc, const char *argv[]);

int runServer(server_t *server);

int initProviderServant();

int initReaderServant();

void* acceptProviders(void *args);

void* acceptReaders(void *args);

// Thread function to serve one provider
void* serveProvider(void *args);

// Thread function to serve the readers
void* serveReaders(void *args);

void showConnectedReaders(list_t *readers);

void showConnectedProviders(list_t *providers);

void destroyServer(server_t *server);

#endif /* end of include guard: SERVER_H */
