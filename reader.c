/*
 *  reader.c
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "reader.h"

int main (int argc, const char *argv[]) {
	reader_t* reader = initReader(argc, argv);
	if (!reader)
		exit(EXIT_FAILURE);
	
	// Avvia il reader	
	int result = runReader(reader);
	
	// Dealloca il reader
	destroyReader(reader);
	
	(result<0)?exit(EXIT_FAILURE):exit(EXIT_SUCCESS);
}

reader_t* initReader(int argc, const char *argv[]) {
	// TODO - Input check
	const char *name = argv[1];
	const char *topic = argv[2];
	
	reader_t *reader = (reader_t *)malloc(sizeof(reader_t));
	reader->name = (char *)malloc(strlen(name)+1);
	strcpy(reader->name, name);
	reader->topic = (char *)malloc(strlen(topic)+1);
	strcpy(reader->topic, topic);
	reader->received = allocList();
	reader->id = 0;
	reader->socket = 0;
	
	return reader;
}

int connectToServer(reader_t *reader) {
	// Ottieni la socket per dialogare con il server
	int sock;
	struct sockaddr_in server;
	struct hostent *hp;
	
	// Ottieni una struttura che descrive il server
	hp = gethostbyname(NEWS_ADDRESS);
	if (hp == NULL) {
		printf("client: Host (%s) è sconosciuto.\n", NEWS_ADDRESS);
		return 0;
	}
	
	// Ottieni la socket dal sistema operativo
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		printf("client: Errore (%s) nella creazione della socket.\n", 
						strerror(errno));
		return 0;
	}
	
	// Inizializza la struttura server
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(NEWS_SUBSCRIBE);
	
	// Connessione
	if (connect(sock, (struct sockaddr *)&server, sizeof(server))<0) {
		printf("client: Errore (%s) durante la connect().\n", strerror(errno));
		return 0;
	}
	
	// La connessione ha avuto successo...
	printf("client: Connesso a (%s) sulla porta %d.\n", NEWS_ADDRESS, 
					ntohs(server.sin_port));
					
	reader->socket = sock;
	return 1;	
}

int subscribeReader(reader_t *reader) {
	char incoming[SIZE_BUFFER];
	
	// 1a) Invia ID e aspetta ACK
	printf("client: Invio al server il topic di interesse (%s).\n", reader->topic);
	if (sendString(reader->socket, reader->topic)<0) {
		printf("client: Errore (%s) durante la write().\n", strerror(errno));
		return 0;	
	}
	
	// 1b) Ricevi ACK (connectionID)
	if (receiveString(reader->socket, incoming, sizeof(incoming))<=0) {
		printf("client: Errore (%s) durante la read().\n", strerror(errno));
		return 0;
	}
	if (atoi(incoming)>0) {
		reader->id = atoi(incoming);
		printf("client: Connessione riuscita! (%d)\n", reader->id);
	} else {
		printf("client: Errore nella comunicazione.\n");
		return 0;
	}
	
	return 1;	
}

int getNews(reader_t *reader) {
	char incoming[SIZE_BUFFER];
	
	// Protocollo di interazione col server:
	//	1) ricevi notizie fino all'invio di un messagio di terminazione.
	printf("client: Inizio ricezione notizie...\n");
	while (1) {
		// Ricevi prossima notizia
		if (receiveString(reader->socket, incoming, sizeof(incoming))<=0){
			printf("client: Errore (%s) durante la read().\n", strerror(errno));
			return 0;
		}

		// Se ricevi un messaggio QUIT, termina
		if (atoi(incoming)==QUIT) {
			printf("client: Flusso terminato.\n", incoming);
			break;
		} else {
			addElement(reader->received, incoming);
			printf("client: Ricevuta notizia [%s].\n", incoming);
		}
	}
	
	printf("client: esco...\n");
	
	return 1;	
}

int runReader(reader_t *reader) {
	if (!connectToServer(reader))
		return 0;
	
	if (!subscribeReader(reader))
		return 0;
	
	return getNews(reader);	
}

void destroyReader(reader_t *reader) {
	free(reader->name);
	free(reader->topic);
	freeList(reader->received);
	free(reader);
}