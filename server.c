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
	// TODO - Fix semaphore problem
	//server->sem_readers = (sem_t *)malloc(sizeof(sem_t));
	//server->sem_providers = (sem_t *)malloc(sizeof(sem_t));
	// sem_init not implemented on Mac OS X, try with named semaphores (sem_open)
	//int r = sem_init(server->sem_readers, 0, 1);
	//server->sem_readers = sem_open("/sreaders", O_CREAT, 0666, 1);
	//if (server->sem_readers==SEM_FAILED)
	//	printf("server: Errore (%s) nella sem_open().\n", strerror(errno));
	//sem_init(server->sem_providers, 0, 1);
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
	pthread_t provider_accepter, reader_accepter, news_dispatcher;
	void *status[3];
	
	pthread_create(&provider_accepter, NULL, &acceptProviders, server);
	pthread_create(&reader_accepter, NULL, &acceptReaders, server);
	pthread_create(&news_dispatcher, NULL, &serveReaders, server);
	
	pthread_join(provider_accepter, status[0]);
	pthread_join(reader_accepter, status[1]);
	pthread_join(reader_accepter, status[2]);
	
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
	
	// Ottengo un riferimento al server
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
		//sem_wait(server->sem_providers);
			int present = isRegistered(server->providers, incoming);
		//sem_post(server->sem_providers);
		if (present) {
			sprintf(response, "%d", NAME_KO);
			if (sendString(sockmsg, response)<0) {
				printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
								processID, threadID, strerror(errno));
				error = 1;
				break;
			}			
			printf("server %d (thread %lu): ID già presente, registrazione rifiutata!\n", 
							processID, threadID, incoming);
			close(sockmsg);
		} else {
			// TODO - Generate serial unique IDs
			int id = 1;
			sprintf(response, "%d", id);
			if (sendString(sockmsg, response)<0) {
				printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
								processID, threadID, strerror(errno));
				error = 1;
				break;
			}
			// Crea un nuovo provider ID
			printf("server %d (thread %lu): ID accettato!\n", processID, threadID);
			pthread_t worker;
			provider_id_t *provider = makeProviderID(server->news, worker, incoming, sockmsg);
			
			//sem_wait(server->sem_providers);
				addElement(server->providers, provider);
				printf("server %d (thread %lu): providers -> ", processID, threadID);
				showConnectedProviders(server->providers);
			//sem_post(server->sem_providers);
			
			// TODO - Start a new thread to server the provider
			pthread_create(&worker, NULL, &serveProvider, provider);
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
	
	// Ottengo un riferimento al server
	server_t* server = (server_t *)args;
	
	while (1) {
		// Aspetta una connessione
		int sockmsg;
		printf("server %d (thread %lu): Attendo connessione...\n", 
						processID, threadID);
		if ((sockmsg = accept(server->socket_readers, NULL, 0))<0) {
			printf("server %d (thread %lu): Errore (%s) nella accept().\n", 
							processID, threadID, strerror(errno));
			error = 1;
			break;
		}
		
		// Connessione avvenuta con successo
		printf("server %d (thread %lu): Connessione effettuata!\n", 
						processID, threadID);
						
		// Preparati a ricevere l'id del news reader
		int len = 0;
		char incoming[SIZE_BUFFER];
		char response[SIZE_BUFFER];
		
		// 1) Ricevi l'id
		len = receiveString(sockmsg, incoming, sizeof(incoming));
		if (len<=0) {
			error = 1;
			break;
		}
		printf("server %d (thread %lu): Registro nuovo reader (%s)...\n", 
						processID, threadID, incoming);

		// TODO - Generate serial unique IDs		
		int id = 1;			
		reader_id_t *reader = makeReaderID(id, incoming, sockmsg);
		//sem_wait(server->sem_readers);
			addElement(server->readers, reader);
			printf("server %d (thread %lu): readers -> ", processID, threadID);
			showConnectedReaders(server->readers);
		//sem_post(server->sem_readers);

		sprintf(response, "%d", id);
		if (sendString(sockmsg, response)<0) {
			printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
							processID, threadID, strerror(errno));
			error = 1;
			break;
		}
	}
	
	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

int isRegistered(list_t *providers, char *new_provider_id) {
	iterator_t *i = createIterator(providers);
	int found = 0;
	
	while (hasNext(i) && !found) {
		provider_id_t *provider_id = (provider_id_t *)next(i);
		if (!strcmp(provider_id->id, new_provider_id))
			found = 1;
	}
	
	return found;
}

void* serveProvider(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();

	char incoming[SIZE_BUFFER];
	char response[SIZE_BUFFER];
	char topic[SIZE_BUFFER];
	int len = 0;
	int ack = 0;
	int error = 0;
		
	// Ottengo un riferiemnto al providerda servire
	provider_id_t *provider = (provider_id_t *)args;
	
	printf("server %d (thread %lu): Servo il provider \"%s\"...\n", 
					processID, threadID, provider->id);

	// Ricevi il topic delle news e manda un ACK
	len = receiveString(provider->socket, incoming, sizeof(incoming));
	if (len<=0)
		error = 1;

	if (!error) {
		// Manda l'ack per il topic
		strcpy(topic, incoming);
		printf("server %d (thread %lu): Ricevuto topic (%s).\n", processID, threadID, incoming);
		sprintf(response, "%d", ack++);
		if (sendString(provider->socket, response)<0)
			error = 1;
		printf("server %d (thread %lu): Rispondo con ACK (%d).\n", processID, threadID, ack);
		
		// Servi il client fino alla disconnessione
		while (1) {
			// Leggi prossimo messaggio
			len = receiveString(provider->socket, incoming, sizeof(incoming));
			if (len<=0) {
				printf("server %d (thread %lu): Client si è disconnesso.\n", processID, threadID);
				break;
			}

			printf("server %d (thread %lu): Ricevuto messaggio (%s).\n", processID, threadID, incoming);
			// Se il messaggio è QUIT abbatti la connessione
			if (!strcmp(incoming,"QUIT"))
				break;

			// Accoda la notizia nel buffer
			int *result;
			int delay = 1;
			int tries = 0;
			
			// Costruisci una struttura notizia
			news_t *news = makeNews(topic, incoming, 1);
			
			result = (int *)putBloccante(provider->buffer, news);
			while (result!=(int *)BUFFER_OK && tries<MAX_TRIES) {
				// Mando al client un segnale di attesa
				sprintf(response, "%d", WAIT);
				if (sendString(provider->socket, response)<0) {
					printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
									processID, threadID, strerror(errno));
					error = 1;
					break;
				}

				// Se l'inserimento non va a buon fine, aspetta un tempo delay e riprova
				tries++;
				printf("server %d (thread %lu): Buffer pieno, aspetto %ds...\n", 
								processID, threadID, delay);
				sleep(delay);
				printf("server %d (thread %lu): Provo di nuovo (Tentativo: %d).\n", 
								processID, threadID, tries);
				result = (int *)putBloccante(provider->buffer, news);
				delay *= 2;		// Aspetta 1s, poi 2s, poi 4s, poi 8s etc.
			}

			if (result==(int *)BUFFER_OK) {
				// Manda un ACK
				sprintf(response, "%d", ack);
				if (sendString(provider->socket, response)<0) {
					printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
									processID, threadID, strerror(errno));
					error = 1;
					break;
				}
				printf("server %d (thread %lu): Rispondo con ACK (%d).\n", 
								processID, threadID, ack++);
			} else {
				// Manda un segnale di abbandono
				sprintf(response, "%d", FULL);
				if (sendString(provider->socket, response)<0) {
					printf("server %d (thread %lu): Errore (%s) durante la write.\n", 
									processID, threadID, strerror(errno));
					error = 1;
					break;
				}
				printf("server %d (thread %lu): Buffer occupato.\n", processID, threadID);
				break;
			}
		}
	}

	close(provider->socket);
	provider->active = 0;
	// TODO - Remove provider from list
	printf("server %d (thread %lu): Chiudo la connessione con \"%s\".\n", 
					processID, threadID, provider->id);

	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void* serveReaders(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();

	char response[SIZE_BUFFER];
	int len = 0;
	int ack = 0;
	int error = 0;
		
	// Ottengo un riferiemnto al providerda servire
	server_t *server = (server_t *)args;
	
	while(1) {
		printf("server %d (thread %lu): Servo i reader...\n", 
						processID, threadID);
		
		// Get a news
		news_t *news = (news_t *)malloc(sizeof(news_t));
		int *result = (int *)getBloccanteB(server->news, (void *)&news);
		while (result==WAIT_MSG) {
			result = (int *)getBloccanteB(server->news, (void *)&news);
		}			

		// Send the news
		//sem_wait(server->sem_readers);
		iterator_t *i = createIterator(server->readers);
		while (hasNext(i) ) {
			reader_id_t *reader = (reader_id_t *)next(i);
			if (reader->active) {
				if (!strcmp(reader->topic, news->topic)) {
					printf("server %d (thread %lu): Mando la notizia al reader (%d).\n", 
									processID, threadID, reader->socket);
					if (sendString(reader->socket, news->message)<0) {
						error = 1;
						break;
					}
				}
			}
		}
		freeIterator(i);
		//sem_post(server->sem_readers);
		
		sleep(1);
	}
	
	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void showConnectedReaders(list_t *readers) {
	iterator_t *i = createIterator(readers);
	printf("{ ");
	while(hasNext(i)) {
		reader_id_t *reader_id = (reader_id_t *)next(i);
		printf("%d ", reader_id->socket);
	}
	printf("}\n");
	freeIterator(i);	
}

void showConnectedProviders(list_t *providers) {
	iterator_t *i = createIterator(providers);
	printf("{ ");
	while(hasNext(i)) {
		provider_id_t *provider_id = (provider_id_t *)next(i);
		printf("%s ", provider_id->id);
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
	//sem_unlink("/sreaders");
	//sem_destroy(server->sem_providers);
	free(server);
}