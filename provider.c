/*
 *  provider.c
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "provider.h"

int main (int argc, const char *argv[])
{
	// TODO - Controlla input da linea di comando
	provider_t* provider = initProvider(argc, argv);
	if (!provider)
		exit(EXIT_FAILURE);
	
	// Avvia il server	
	int result = runProvider(provider);
	
	// Dealloca il server
	destroyProvider(provider);
	
	(result<0)?exit(EXIT_FAILURE):exit(EXIT_SUCCESS);
}

provider_t* initProvider(int argc, const char *argv[]) {
	// TODO - Controlla input da linea di comando
	const char *name = argv[1];
	const char *topic = argv[2];
	int msgs = atoi(argv[3]);
	int delay = atoi(argv[4]);
	int live = 0;
	
	provider_t *provider = (provider_t *)malloc(sizeof(provider_t));
	provider->name = (char *)malloc(strlen(name)+1);
	strcpy(provider->name, name);
	provider->topic = (char *)malloc(strlen(topic)+1);
	strcpy(provider->topic, topic);
	provider->msgs = msgs;
	provider->delay = delay;
	provider->live = live;
	provider->socket = 0;
	
	return provider;
}

int connectToServer(provider_t *provider) {
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
	if ((sock = socket(AF_INET, SOCK_STREAM, 0))<0) {
		printf("client: Errore (%s) nella creazione della socket.\n", 
						strerror(errno));
		return 0;
	}
	
	// Inizializza la struttura server
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(NEWS_PUBLISH);
	
	// Connessione
	if (connect(sock, (struct sockaddr *)&server, sizeof(server))<0) {
		printf("client: Errore (%s) durante la connect.\n", strerror(errno));
		return 0;
	}
	
	// La connessione ha avuto successo...
	printf("client: Connesso a (%s), sulla porta %d.\n", NEWS_ADDRESS, 
					ntohs(server.sin_port));
					
	provider->socket = sock;
	return 1;
}

int advertiseProvider(provider_t *provider) {
	char incoming[SIZE_BUFFER];
	
	// 1a) Invia ID e aspetta ACK
	printf("client: Invio ID (%s)\n", provider->name);
	if (sendString(provider->socket, provider->name)<0) {
		printf("client: Errore (%s) durante la write\n", strerror(errno));
		return 0;	
	}
	
	// 1b) Ricevi ACK (connectionID)
	if (receiveString(provider->socket, incoming, sizeof(incoming))<=0) {
		printf("client: Errore %s durante la read\n", strerror(errno));
		return 0;
	}
	if (atoi(incoming)==NAME_KO) {
		printf("client: ID già in uso!\n");
		return 0;
	} else if (atoi(incoming)>0) {
		provider->id = atoi(incoming);
		printf("client: Connessione riuscita! (%d)\n", provider->id);
	} else {
		printf("client: Errore nella comunicazione\n");
		return 0;
	}
	
	return 1;
}

int sendNews(provider_t *provider) {
	while (1) {
		printf("client: Zzzzzzzzzz...\n");
		sleep(10);
	}
	// Protocollo di interazione col server:
	//	1) manda ID --> ricevi sonnection ID
	//	2) manda topicID --> ricevi ACK
	//  3) finchè ci sono messaggi:
	//		31. manda messaggio --> ricevi ACK
	//  4) se richiesto ricomincia da 4, sennò termina.
	char incoming[SIZE_BUFFER];
	char outgoing[SIZE_BUFFER];
	int toack = 0;

	// 2a) Manda il topic ID
	printf("client: Invio al server il topicID=\"%s\"\n", provider->topic);
	if (sendString(provider->socket, provider->topic)<0) {
		printf("client: Errore (%s) durante la write().\n", strerror(errno));
		return 0;
	}
	// 2b) Ricevi ACK (Se non ricevi un ACK c'è un errore, termina)
	if (receiveString(provider->socket, incoming, sizeof(incoming))<=0) {
		printf("client: Errore (%s) durante la read().\n", strerror(errno));
		return 0;
	}
	if (atoi(incoming)!=toack++) {
		printf("client: Errore nella comunicazione.\n");
		return 0;
	}
	printf("client: Ricevuto ACK (%s), inizio spedizione notizie...\n", incoming);

	// 3a) Manda msgs messaggi ad intervallo delay
	int i = provider->msgs;
	while (1) {
		while ((i--)>0) {
			sprintf(outgoing, "%d", provider->msgs-i);
			printf("client: Invio al server il messaggio %s.\n", outgoing);
			if (sendString(provider->socket, outgoing)<0) {
				printf("client: Errore (%s) durante la write().\n", strerror(errno));
				return 0;
			}
			// Ricevi ACK
			if (receiveString(provider->socket, incoming, sizeof(incoming))<=0){
				printf("client: Errore (%s) durante la read().\n", strerror(errno));
				return 0;
			}

			// Se non ricevi un ACK c'è un errore, termina
			// L'ack per il messaggio 0 è 1, per il messaggio 1 è 2 etc.
			if (atoi(incoming)==toack) {
				printf("client: Ricevuto ACK (%s).\n", incoming);
				toack++;
			} else if (atoi(incoming)==0) {
				// Attendo ricezione ACK
				while (atoi(incoming)==0) {
					printf("client: Attendo risposta dal server...\n");
					if (receiveString(provider->socket, incoming, sizeof(incoming))<=0){
						printf("client: Errore (%s) durante la read().\n", strerror(errno));
						return 0;
					}
				}
				if (atoi(incoming)==toack) {
					printf("client: Ricevuto ACK (%s)\n", incoming);
					toack++;
				} else {
					printf("client: Riprova più tardi...\n");
					return 0;
				}
			} else {
				printf("client: Errore nella comunicazione.\n");
				return 0;
			}
			
			sleep(provider->delay);
		}

		// Conitnua o no?
		if (provider->live) {
			char input;
			printf("client: Notizie esaurite... continuo? (N or n to quit) ");
			while ((input = getchar()) == '\n')
				;
			if (input=='N' || input=='n') {
				printf("client: Chiudo la connessione.\n");
				if (sendString(provider->socket, (char *)-4)<0) {
					printf("Errore (%s) durante la write().\n", strerror(errno));
					return 0;
				}
				break;
			}
			i += provider->msgs;
		} else {
			printf("client: notizie esaurite\n");
			if (sendString(provider->socket, (char *)-4)<0) {
				printf("client: errore (%s) durante la write\n", strerror(errno));
				return 0;
			}
			break;
		}
	}
	
	return 1;
}

int runProvider(provider_t *provider) {
	if (!connectToServer(provider))
		return 0;
	
	if (!advertiseProvider(provider))
		return 0;
	
	return sendNews(provider);
}

void destroyProvider(provider_t *provider) {
	
}