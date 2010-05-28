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
	//reader->running = 1;
	
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
	
	// Invia ID e aspetta ACK
	printf("client: Invio ID (%s).\n", reader->name);
	if (sendString(reader->socket, reader->name)<0) {
		printf("client: Errore (%s) durante la write().\n", strerror(errno));
		return 0;	
	}
	
	// Ricevi ACK (connectionID)
	if (receiveString(reader->socket, incoming, sizeof(incoming))<=0) {
		printf("client: Errore (%s) durante la read().\n", strerror(errno));
		return 0;
	}
	if (atoi(incoming)>0) {
		reader->id = atoi(incoming);

		// Invia Topic e aspetta ACK
		printf("client: Invio al server il topic (%s)\n", reader->topic);
		if (sendString(reader->socket, reader->topic)<0) {
			printf("client: Errore (%s) durante la write().\n", strerror(errno));
			return 0;
		}
		// Ricevi ACK (Se non ricevi un ACK c'è un errore, termina)
		if (receiveString(reader->socket, incoming, sizeof(incoming))<=0) {
			printf("client: Errore (%s) durante la read().\n", strerror(errno));
			return 0;
		}
		if (atoi(incoming)==0) {
			printf("client: Connessione riuscita! (%d)\n", reader->id);
		} else {
			printf("client: Errore nella comunicazione.\n");
			return 0;
		}
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
	printf("client: Inizio ricezione notizie... (Premi 'q' e invio per uscire)\n");
	printf("client: Formato notizie -> [id:provider~message].\n");
	
	// Start a thread to get input commands
	// pthread_t input_getter;
	// void *status;
	// pthread_create(&input_getter, NULL, &getInput, reader);
	
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
	
	//pthread_join(input_getter, status);
	printf("client: esco...\n");
	
	//if (status != SUCCESS)
	//	return 0;
		
	return 1;
}

// void* getInput(void *args) {
// 	int error = 0;
// 	
// 	// Ottieni il riferimento al client reader
// 	reader_t *reader = (reader_t *)args;
// 	
// 	char input;
// 	while ((input = getchar()) == '\n')
// 		;
// 	
// 	if (input!='q') {
// 		printf("client: Input non valido. Premi 'q' e invio per chiudere.\n");
// 		scanf("%s", input);
// 	}
// 	reader->running = 0;
// 	
// 	// Send a quit message
// 	if (sendString(reader->socket, "QUIT")<0) {
// 		printf("client: Errore (%s) durante la write().\n", strerror(errno));
// 		error = 1;
// 	}
// 	printf("client: Chiudo la connessione.\n");
// 	
// 	destroyReader(reader);
// 	
// 	if (error)
// 		exit(EXIT_FAILURE);
// 	else
// 		exit(EXIT_SUCCESS);
// }

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