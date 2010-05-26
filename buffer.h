/*
 * buffer.h
 *
 * This file defines a thread-safe interface of a dynamic-sized buffer.
 *
 *  Created on: 13/05/2010
 *  Author: matte
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

// Definitions
#define WAIT_MSG			(void *) -4
#define POISON_PILL 	(void *) -3
#define BUFFER_PIENO 	(void *) -2
#define BUFFER_VUOTO 	(void *) -1
#define BUFFER_OK 	  (void *)  0
#define DEF_MSG				(void *)  1

#define MAX_BUF_LEN 					 64			// Massima capacità del buffer
#define MAX_WAIT 							 15			// Massima attesa delle funzioni bloccanti in secondi

typedef struct buffer_t {
	void* buffer[MAX_BUF_LEN];				// Il buffer che contiene i messaggi
	pthread_mutex_t *buffer_mutex;			// Mutex per l'accesso al buffer
	pthread_cond_t  *buffer_non_pieno_cv;	// Condizione buffer non pieno
	pthread_cond_t  *buffer_non_vuoto_cv;	// Condizione buffer non vuoto
	int N;									// Numero massimo meessaggi
	int T;									// Indice prossima estrazione
	int D;									// Indice prossimo inserimento
	int K;									// Numero di messaggi attualmente nel buffer
} buffer_t;

/* 
 * Inserimento bloccante: sospende se pieno, quindi
 * effettua l'inserimento non appena si libera dello spazio
 */
void* putBloccante(buffer_t* buffer, void *msg);

/* 
 * Estrazione bloccante: sospende se vuoto, quindi
 * restituisce il valore estratto non appena disponibile
 */
void* getBloccante(buffer_t* buffer);

void* getBloccanteB(buffer_t* buffer, void **output);

/*
 * Inizializza il buffer vuoto con la capacità specificata
 * nella variabile capacity, ritorna un puntatore
 * alla struttura buffer_t inizializzata
 */
buffer_t* initVuoto(int capacity);

/*
 * Inizializza il buffer pieno (con messaggi di default DEF_MSG)
 * con la capacità specificata nella variabile capacity, 
 * ritorna un puntatore alla struttura buffer_t inizializzata
 */
buffer_t* initPieno(int capacity);

/* Dealloca la memoria usata dal buffer */
void destroyBuffer(buffer_t* buffer);

#endif /* end of include guard: BUFFER_H */
