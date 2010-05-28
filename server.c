/*
 *  server.c
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "server.h"

int main (int argc, const char *argv[]) {
	server_t* server = initServer(argc, argv);
	if (!server)
		exit(EXIT_FAILURE);
	
	// Avvia il server	
	int result = runServer(server);
	
	// Dealloca il server
	destroyServer(server);
	
	if(result) {
		printf("L'esecuzione ha avuto successo.\n");
		exit(EXIT_SUCCESS);
	 } else {
		printf("Riscontrati errori d'esecuzione.\n");
		exit(EXIT_FAILURE);
	}
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
	// Using mutex, works everywhere
	//server->readers_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	//pthread_mutex_init(server->readers_mutex, NULL);
	//server->providers_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	//pthread_mutex_init(server->providers_mutex, NULL);
	// Using unnamed semaphores (unnamed semaphores not implemented on Mac OS)
	//server->sem_readers = (sem_t *)malloc(sizeof(sem_t));
	//server->sem_providers = (sem_t *)malloc(sizeof(sem_t));
	//int s1 = sem_init(server->sem_readers, 0, 1);
	//int s2 = sem_init(server->sem_readers, 0, 1);
	//if (s1<0 || s2<0)
	//	printf("server: Errore (%s) nella sem_open().\n", strerror(errno));
	// Using named semaphores, seems to work everywhere
	server->sem_readers = sem_open("/sreaders", O_CREAT, 0666, 1);
	server->sem_providers = sem_open("/sproviders", O_CREAT, 0666, 1);
	if (server->sem_readers==SEM_FAILED ||
			server->sem_providers==SEM_FAILED) {
		printf("server: Errore (%s) nella sem_open().\n", strerror(errno));
		return NULL;
	}
	
	return server;
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
		printf("server %d: setsockopt() fallita.\n", getpid());
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
		printf("server %d: setsockopt() fallita\n", getpid());
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
	pthread_t provider_cleaner, reader_cleaner;
	void *status[5];
	
	pthread_create(&provider_accepter, NULL, &acceptProviders, server);
	pthread_create(&reader_accepter, NULL, &acceptReaders, server);
	pthread_create(&news_dispatcher, NULL, &serveReaders, server);
	pthread_create(&provider_cleaner, NULL, &cleanProviders, server);
	pthread_create(&reader_cleaner, NULL, &cleanReaders, server);
	
	pthread_join(provider_accepter, status[0]);
	pthread_join(reader_accepter, status[1]);
	pthread_join(news_dispatcher, status[2]);
	pthread_join(provider_cleaner, status[3]);
	pthread_join(reader_cleaner, status[4]);
	
	int i;
	for (i=0; i<5; i++) {
		if (status[i] != SUCCESS) {
			switch (i) {
				case 0:
					printf("Errore nell'accettazione provider...\n");
					break;
				case 1:
					printf("Errore nell'accettazione reader...\n");
					break;
				case 2:
					printf("Errore nello smistamento notizie...\n");
					break;
				case 3:
					printf("Errore nella pulizia provider...\n");
					break;
				case 4:
					printf("Errore nella pulizia reader...\n");
					break;
				default:
					break;			
			}
			return 0;
		}
	}
	
	return 1;
}

void* acceptProviders(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();
	int error = 0;
	static int serialids = 0;
	
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
		printf("server %d (thread %lu): Provider connesso!\n", 
						processID, threadID);
		
		// Preparati a ricevere l'id del news provider
		char incoming[SIZE_BUFFER];
		char response[SIZE_BUFFER];
		char id[SIZE_BUFFER];
		char topic[SIZE_BUFFER];
		int len = 0;
		
		// Aggiungi nuovo provider alla lista provider:
		//		1) Ricevi l'id del client, verifica se già presente:
		//				1a. Se presente rifiuta e chiudi la connessione;
		//				1b. Sennò accettalo e aggiungilo alla lista.
		//		2) Ricevi il topic delle news e registra il provider
		
		// 1) Ricevi l'id
		len = receiveString(sockmsg, incoming, sizeof(incoming));
		if (len<=0) {
			error = 1;
			break;
		}
		printf("server %d (thread %lu): Controllo ID (%s)...\n", 
						processID, threadID, incoming);
		
		// 1) Verifica presenza
		strcpy(id, incoming);
		sem_wait(server->sem_providers);
		//pthread_mutex_lock(server->providers_mutex);
			int present = isRegistered(server->providers, id);
		//pthread_mutex_unlock(server->providers_mutex);
		sem_post(server->sem_providers);
						
		if (present) {
			sprintf(response, "%d", NAME_KO);
			if (sendString(sockmsg, response)<0) {
				printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
								processID, threadID, strerror(errno));
				error = 1;
				break;
			}			
			printf("server %d (thread %lu): ID già presente, registrazione rifiutata!\n", 
							processID, threadID);
			close(sockmsg);
		} else {
			// Manda l'id della nuova connessione
			sprintf(response, "%d", ++serialids);
			if (sendString(sockmsg, response)<0) {
				printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
								processID, threadID, strerror(errno));
				error = 1;
				break;
			}
			
			// Ricevi il topic delle news e manda un ACK
			len = receiveString(sockmsg, incoming, sizeof(incoming));
			if (len<=0) {
				error = 1;
				break;
			}
			// Manda l'ack per il topic
			strcpy(topic, incoming);
			printf("server %d (thread %lu): Ricevuto da \"%s\" topic (%s).\n", 
							processID, threadID, id, topic);
			sprintf(response, "0");
			if (sendString(sockmsg, response)<0) {
				error = 1;
				break;
			}
			printf("server %d (thread %lu): Rispondo a \"%s\" con ACK (0).\n", 
							processID, threadID, id);
			
			// Crea un nuovo provider ID
			printf("server %d (thread %lu): ID accettato!\n", processID, threadID);
			pthread_t worker;
			provider_id_t *provider = 
				makeProviderID(server->news, worker, id, topic, serialids, sockmsg);
			
			sem_wait(server->sem_providers);
			//pthread_mutex_lock(server->providers_mutex);
				addElement(server->providers, provider);
				printf("server %d (thread %lu): providers -> ", processID, threadID);
				showConnectedProviders(server->providers);
			//pthread_mutex_unlock(server->providers_mutex);
			sem_post(server->sem_providers);
			
			// Avvia un nuovo thread per servire il provider
			// TODO - Capire dove fare la join di tuti questi processi
			// (forse alla fine del while...)
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
	static int serialids = 0;
	
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
		printf("server %d (thread %lu): Reader connesso!\n", 
						processID, threadID);
						
		// Preparati a ricevere l'id del news reader
		int len = 0;
		char incoming[SIZE_BUFFER];
		char response[SIZE_BUFFER];
		char id[SIZE_BUFFER];
		char topic[SIZE_BUFFER];
		
		// 1) Ricevi l'id
		len = receiveString(sockmsg, incoming, sizeof(incoming));
		if (len<=0) {
			error = 1;
			break;
		}
		strcpy(id, incoming);

		// Manda l'id della nuova connessione						
		sprintf(response, "%d", ++serialids);
		if (sendString(sockmsg, response)<0) {
			printf("server %d (thread %lu): Errore (%s) durante la write().\n", 
				processID, threadID, strerror(errno));
			error = 1;
			break;
		}

		// Ricevi il topic delle news e manda un ACK
		len = receiveString(sockmsg, incoming, sizeof(incoming));
		if (len<=0) {
			error = 1;
			break;
		}
		// Manda l'ack per il topic
		strcpy(topic, incoming);
		printf("server %d (thread %lu): Ricevuto da \"%s\" topic (%s).\n", 
						processID, threadID, id, incoming);
		sprintf(response, "0");
		if (sendString(sockmsg, response)<0) {
			error = 1;
			break;
		}
		printf("server %d (thread %lu): Rispondo a \"%s\" con ACK (0).\n", 
			processID, threadID, id);

		printf("server %d (thread %lu): Registro nuovo reader (%s)...\n", 
			processID, threadID, id);

		pthread_t worker;			
		reader_id_t *reader = makeReaderID(worker, id, topic, ++serialids, sockmsg);
		sem_wait(server->sem_readers);
		//pthread_mutex_lock(server->readers_mutex);
			addElement(server->readers, reader);
			printf("server %d (thread %lu): readers -> ", processID, threadID);
			showConnectedReaders(server->readers);
		//pthread_mutex_unlock(server->readers_mutex);
		sem_post(server->sem_readers);

		// TODO - Avvia un thread di ascolto del reader (per gestire disconnessioni...)
		// TODO2 - Capire dove fare il join di questi thread
		// (forse fuori dal while anche qui...)
		pthread_create(&worker, NULL, &serveReader, reader);
	}
	
	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void* serveProvider(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();

	char incoming[SIZE_BUFFER];
	char response[SIZE_BUFFER];

	int len = 0;
	int ack = 1;
	int error = 0;
	static int serialnumber = 0;
		
	// Ottengo un riferiemnto al provider da servire
	provider_id_t *provider = (provider_id_t *)args;
	
	printf("server %d (thread %lu): Servo il provider \"%s\"...\n",  
					processID, threadID, provider->name);
					
	//printf("buffer N: %d\n", provider->buffer->N);
					
	// Servi il client fino alla disconnessione
	while (1) {
		// Leggi prossimo messaggio
		len = receiveString(provider->socket, incoming, sizeof(incoming));
		if (len<=0) {
			printf("server %d (thread %lu): Client si è disconnesso.\n", 
							processID, threadID);
			break;
		}

		printf("server %d (thread %lu): Ricevuto da \"%s\" messaggio (%s).\n", 
						processID, threadID, provider->name, incoming);
		// Se il messaggio è QUIT abbatti la connessione
		if (!strcmp(incoming,"QUIT"))
			break;

		// Accoda la notizia nel buffer
		int *result;
		int delay = 1;
		int tries = 0;

		// Costruisci una struttura notizia
		news_t *news = makeNews(provider->name, provider->topic, 
														incoming, ++serialnumber);

		result = (int *)putBloccante(provider->buffer, news);
		//printf("response: %d\n", result);
		//printf("buffer K: %d\n", provider->buffer->K);
		//printf("buffer N: %d\n", provider->buffer->N);
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
			printf("received message: %d\n", result);
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

	provider->active = 0;
	printf("server %d (thread %lu): Chiudo la connessione con \"%s\".\n", 
					processID, threadID, provider->name);

	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void* serveReader(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();

	char incoming[SIZE_BUFFER];
	int len = 0;
	int error = 0;
	
	reader_id_t *reader = (reader_id_t *)args;
	
	while (1) {
		// Leggi prossimo messaggio
		len = receiveString(reader->socket, incoming, sizeof(incoming));
		if (len<=0) {
			printf("server %d (thread %lu): Client si è disconnesso.\n", 
							processID, threadID);
			break;
		}
		if (!strcmp(incoming,"QUIT"))
			break;
		else {
			printf("server %d (thread %lu): Comando non riconosciuto.\n", 
							processID, threadID);			
		}
		
		sleep(10);
	}
	
	reader->active = 0;
	printf("server %d (thread %lu): Chiudo la connessione con \"%s\".\n", 
					processID, threadID, reader->name);

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
		
	// Ottengo un riferiemnto al server
	server_t *server = (server_t *)args;
	
	while(1) {
		// CHECK - Se ci sono reader attivi procedi (bisogna farlo?)
		// Perchè così i reader ricevono anche i messaggi precedenti
		// alla connessione e questo viola la specifica...
		//int empty = 0;
		//sem_wait(server->sem_readers);
		//	empty = (count(server->readers)==0);
		//sem_post(server->sem_readers);
		
		// Estrai una notizia
		//if (!empty) {
			// CHECK - Così se non ci sono readrs collegati "butto" la notizia...
			news_t *news = (news_t *)malloc(sizeof(news_t));
			int *result = (int *)getBloccanteB(server->news, (void *)&news);
			while (result==BUFFER_WAIT) {
				result = (int *)getBloccanteB(server->news, (void *)&news);
			}			
			

			// Manda la notizia ai reader registrati e attivi
			sem_wait(server->sem_readers);
			//pthread_mutex_lock(server->readers_mutex);
				iterator_t *i = createIterator(server->readers);
				while (hasNext(i) ) {
					reader_id_t *reader = (reader_id_t *)next(i);
					if (reader->active) {
						if (!strcmp(reader->topic, news->topic)) {
							printf("server %d (thread %lu): Mando la notizia al reader (%s).\n", 
											processID, threadID, reader->name);
							char* message = news2Message(news);			
							if (sendString(reader->socket, message)<0) {
								error = 1;
								break;
							}
							free(message);
						}
					}
				}
				freeIterator(i);
			//pthread_mutex_unlock(server->readers_mutex);
			sem_post(server->sem_readers);
		//}
		
		sleep(1);
	}
	
	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void* cleanProviders(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();

	int error = 0;

	// Ottengo un riferiemnto al server
	server_t *server = (server_t *)args;
	
	while (1) {
		int cancel = 0;
		list_t *matches = allocList();
		
		sem_wait(server->sem_providers);
		//pthread_mutex_lock(server->providers_mutex);
			// Cerca provider non attivi
			iterator_t *i = createIterator(server->providers);
			while (hasNext(i)) {
				provider_id_t *provider = (provider_id_t *)next(i);
				if (!provider->active) {
					addElement(matches, provider);
				}
			}
			freeIterator(i);
			
			// Elimina provider non attivi
			iterator_t *j = createIterator(matches);
			while (hasNext(j)) {
				if (!cancel) cancel = 1 - cancel;
				
				void *status;
				provider_id_t *match = (provider_id_t *)next(i);
				printf("server %d (thread %lu): Rilascio provider \"%s\"...\n", 
								processID, threadID, match->name);
				removeElement(server->providers, match);
				
				pthread_join(match->worker, status);
				if (status!=SUCCESS)
					error = 1;
				
				destroyProviderID(match);
			}
			
			if (cancel) {
				printf("server %d (thread %lu): providers -> ", processID, threadID);
				showConnectedProviders(server->providers);
			}
		//pthread_mutex_unlock(server->providers_mutex);
		sem_post(server->sem_providers);

		// Esegui pulizia ogni 10s
		freeList(matches);
		sleep(10);
	}
	
	if (error)
		pthread_exit(FAILURE);
	else
		pthread_exit(SUCCESS);
}

void* cleanReaders(void *args) {
	long threadID = (long)pthread_self();
	int processID = (int)getpid();

	int error = 0;
		
	// Ottengo un riferiemnto al server
	server_t *server = (server_t *)args;
	
	while (1) {
		int cancel = 0;
		list_t *matches = allocList();
		
		sem_wait(server->sem_readers);
		//pthread_mutex_lock(server->readers_mutex);
			// Cerca provider non attivi
			iterator_t *i = createIterator(server->readers);
			while (hasNext(i)) {
				reader_id_t *reader = (reader_id_t *)next(i);
				if (!reader->active) {
					addElement(matches, reader);
				}
			}
			freeIterator(i);
			
			// Elimina reader non attivi
			iterator_t *j = createIterator(matches);
			while (hasNext(j)) {
				if (!cancel) cancel = 1 - cancel;
				
				void *status;
				reader_id_t *match = (reader_id_t *)next(i);
				printf("server %d (thread %lu): Rilascio provider \"%s\"...\n", 
								processID, threadID, match->name);
				removeElement(server->readers, match);
				
				pthread_join(match->worker, status);
				if (status!=SUCCESS)
					error = 1;
				
				destroyReaderID(match);
			}
			
			if (cancel) {
				printf("server %d (thread %lu): readers -> ", processID, threadID);
				showConnectedProviders(server->readers);
			}
		//pthread_mutex_unlock(server->readers_mutex);
		sem_post(server->sem_readers);
		
		// Esegui ogni 10s
		freeList(matches);	
		sleep(10);
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
		printf("%s ", reader_id->name);
	}
	printf("}\n");
	freeIterator(i);	
}

void showConnectedProviders(list_t *providers) {
	iterator_t *i = createIterator(providers);
	printf("{ ");
	while(hasNext(i)) {
		provider_id_t *provider_id = (provider_id_t *)next(i);
		printf("%s ", provider_id->name);
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
	//pthread_mutex_destroy(server->readers_mutex);
	//pthread_mutex_destroy(server->providers_mutex);
	sem_destroy(server->sem_readers);
	sem_destroy(server->sem_providers);
	free(server);
}

int isRegistered(list_t *providers, char *new_provider_id) {
	iterator_t *i = createIterator(providers);
	int found = 0;

	while (hasNext(i) && !found) {
		provider_id_t *provider_id = (provider_id_t *)next(i);
		if (!strcmp(provider_id->name, new_provider_id))
			found = 1;
	}
	
	return found;
}