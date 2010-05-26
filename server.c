/*
 *  server.c
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "server.h"

int main (int argc, const char *argv[])
{
	server_t* server = initServer(argc, argv);
	if (!server)
		exit(EXIT_FAILURE);
	
	// Avvia il server	
	int result = runServer(server);
	
	// Dealloca il server
	destroyServer(server);
	
	result?exit(EXIT_FAILURE):exit(EXIT_SUCCESS);
}

server_t* initServer(int argc, const char *argv[]) {
	// TODO - Controlla input da linea di comando
	server_t* server = (server_t *)malloc(sizeof(server_t));
	
	// Inizializza le socket
	int socket_providers = initProviderServant();
	int socket_readers = initReaderServant();
	if (!(socket_providers || socket_readers))
		return NULL;

	server->socket_providers = socket_providers;
	server->socket_readers = socket_readers;
	server->news = initVuoto(SIZE_NEWS);
	server->readers = allocList();
	server->providers = allocList();
}

int initProviderServant() {
	// Variabili per la comunicazione via socket
	int sock_w;
	struct sockaddr_in server_w;
	
	// Ottieni una socket dal SO
	sock_w = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_w<0) {
		printf("server %d: errore (%s) nella creazione la socket.\n",
						getpid(), strerror(errno));
		return 0;
	}
	
	// Il server può riusare porte già usate in precedenza
	int reuse = 1;
	if (setsockopt(sock_w, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(int))<0) {
		printf("server %d: setsockopt() fallita.\n");
		return 0;
	}
	
	// Inizializza il server per la comunicazione (writers)
	server_w.sin_family = AF_INET;
	server_w.sin_addr.s_addr = INADDR_ANY;
	server_w.sin_port = htons(NEWS_PUBLISH);
	
	// Esegui la bind() sulla socket modello
	if (bind(sock_w, (struct sockaddr *)&server_w, sizeof(server_w))<0) {
		printf("server %d: bind() fallita.\n", getpid());
		return 0;
	}
	
	// La bind() ha avuto successo
	printf("server %d: Ascolto sulla porta %d...\n", getpid(),
					ntohs(server_w.sin_port));
	
	if (listen(sock_w, BACKLOG_SIZE)<0) {
		printf("server %d: errore (%s) nella listen.\n", getpid(), strerror(errno));
		return 0;
	}
	
	return sock_w;
}

int initReaderServant() {
	// Variabili per la comunicazione via socket
	int sock_r;
	struct sockaddr_in server_r;
	
	// Ottieni una socket dal SO
	sock_r = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_r<0) {
		printf("server %d: errore %s nel creare la socket\n",
						getpid(), strerror(errno));
		return 0;
	}
	
	// Il server può riusare porte già usate in precedenza
	int reuse = 1;
	if (setsockopt(sock_r, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(int))<0) {
		printf("server %d: setsockopt() fallita\n");
		return 0;
	}
	
	// Inizializza il server per la comunicazione (readers)
	server_r.sin_family = AF_INET;
	server_r.sin_addr.s_addr = INADDR_ANY;
	server_r.sin_port = htons(NEWS_SUBSCRIBE);
	
	// Esegui la bind() sulla socket modello
	if (bind(sock_r, (struct sockaddr *)&server_r, sizeof(server_r))<0) {
		printf("server %d: bind() fallita.\n", getpid());
		return 0;
	}
	
	// La bind() ha avuto successo
	printf("server %d: Ascolto sulla porta %d...\n", getpid(),
					ntohs(server_r.sin_port));
	
	if (listen(sock_r, BACKLOG_SIZE)<0) {
		printf("server %d: errore %s nella listen.\n", getpid(),
						strerror(errno));
		return 0;
	}
	
	return sock_r;	
}

int runServer(server_t *server) {
	pthread_t provider_accepter, reader_accepter;//, news_dispatcher;
	void *status[3];
	
	pthread_create(&provider_accepter, NULL, &acceptProviders, server);
	pthread_create(&reader_accepter, NULL, &acceptReaders, server);
	//pthread_create(&news_dispatcher, NULL, &dispatch, server->dispatcher);
	
	pthread_join(provider_accepter, status[0]);
	pthread_join(reader_accepter, status[1]);
	//pthread_join(reader_accepter, status[2]);
	
	int i;
	for (i=0; i<3; i++) {
		if (status[i] != SUCCESS)
			return 0;
	}
	
	return 1;
}

void* acceptProviders(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();
	int error = 0;
	
	// Ottengo un riferimento all'accepter
	server_t* server = (server_t *)args;
	
	// Ciclo infinito
	while (1) {
		// Aspetta una connessione
		int sockmsg;
		printf("server %d (thread %lu): Attendo connessione...\n", 
						processID, threadID);
		if ((sockmsg = accept(server->socket_providers, NULL, 0))<0) {
			printf("server %d (thread %lu): Errore (%s) nella accept().\n", 
							processID, threadID, strerror(errno));
			error = 1;
			break;
		}
		
		// Connessione avvenuta con successo
		printf("server %d (thread %lu): Connessione effettuata!\n", 
						processID, threadID);
		
		// TODO - Usa un altro thread per gestire la connessione con un nuovo client
		// Preparati a ricevere l'id del news provider
		int len = 0;
		char incoming[SIZE_BUFFER];
		char response[SIZE_BUFFER];
		
		// Aggiungi nuovo provider alla lista provider:
		//		1) Ricevi l'id del client, verifica se già presente:
		//				1a. Se presente rifiuta e chiudi la connessione;
		//				1b. Sennò accettalo e aggiungilo alla lista.
		
		// 1) Ricevi l'id
		len = receiveString(sockmsg, incoming, sizeof(incoming));
		if (len<=0) {
			error = 1;
			break;
		}
		printf("server %d (thread %lu): Controllo ID (%s)...\n", 
						processID, threadID, incoming);
		
		// 1) Verifica presenza
		int present = 0;		// TODO - Controlla presenza realmente
		if (present) {
			sprintf(response, "%d", NAME_KO);
			if (sendString(sockmsg, response)<0) {
				printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
								processID, threadID, strerror(errno));
				error = 1;
				break;
			}			
			printf("server %d (thread %lu): ID già presente, connessione rifiutata!\n", 
							processID, threadID, incoming);
			close(sockmsg);
		} else {
			int id = 1;
			sprintf(response, "%d", id);
			if (sendString(sockmsg, response)<0) {
				printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
								processID, threadID, strerror(errno));
				error = 1;
				break;
			}
			addElement(server->providers, incoming);
			printf("server %d (thread %lu): ID accettato!\n", processID, threadID);
			printf("server %d (thread %lu): providers -> ", processID, threadID);
			showConnectedProviders(server->providers);
			
			// TODO - Start a new thread to server the provider
		}
	}
	
	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void* acceptReaders(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();
	int error = 0;
	
	// Ottengo un riferimento all'accepter
	server_t* server = (server_t *)args;
	
	while (1) {
		printf("server %d (thread %lu): Zzzzzzzzzz...\n", processID, threadID);
		sleep(20);
	}	
}

void* serverProvider(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();
	int error = 0;
	
	// Ottengo un riferiemnto al providerda servire

	
	while(1) {
		printf("server %d (thread %lu): Servo un provider...\n", processID, threadID);
		sleep(2);
	}
}

void* serveReaders(void *args) {
	// TODO
}

void showConnectedReaders(list_t *readers) {
	iterator_t *i = createIterator(readers);
	printf("{ ");
	while(hasNext(i)) {
		char *current_id = (char *)next(i);
		printf("%s ", current_id);
	}
	printf("}\n");
	freeIterator(i);	
}

void showConnectedProviders(list_t *providers) {
	iterator_t *i = createIterator(providers);
	printf("{ ");
	while(hasNext(i)) {
		char *current_id = (char *)next(i);
		printf("%s ", current_id);
	}
	printf("}\n");
	freeIterator(i);
}

void destroyServer(server_t *server) {
	close(server->socket_providers);
	close(server->socket_readers);
	destroyBuffer(server->news);
	freeList(server->providers);
	freeList(server->providers);
	free(server);
}