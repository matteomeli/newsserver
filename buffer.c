/*
 * buffer.c
 * 
 * This file defines a thread-safe implementation of a dynamic-sized buffer.
 *
 *  Created on: 13/5/2010
 *  Author: matte
 */

#include "buffer.h"
#include <time.h>
#include <errno.h>

void* putBloccante(buffer_t* buffer, void *msg) {
	// Usa un timer per evitare attese infinite
	struct timespec timer;
	struct timeval now;
	
	// Ottieni il lock sulla risorsa
	pthread_mutex_lock(buffer->buffer_mutex);
		while(buffer->K==buffer->N) {
			// Inizializza il timer
			gettimeofday(&now, NULL);
			timer.tv_sec = now.tv_sec;
			timer.tv_nsec = now.tv_usec * 1000;
			timer.tv_sec += MAX_WAIT;
			
			// Attendi che il buffer non sia pieno per un tempo max di MAX_WAIT sec
			if (pthread_cond_timedwait(buffer->buffer_non_pieno_cv, buffer->buffer_mutex, &timer) == ETIMEDOUT) {
				// Superato il tempo massimo ->
				// rilascia il lock, annulla operazione e ritorna un codice d'errore
				pthread_mutex_unlock(buffer->buffer_mutex);
				return WAIT_MSG;
			}
		}

		// Esegui operazione
		buffer->buffer[buffer->D] = msg;
		buffer->D = (buffer->D + 1) % buffer->N;
		buffer->K = buffer->K + 1;

		// Segnala il "riempimento" del buffer
		pthread_cond_signal(buffer->buffer_non_vuoto_cv);
	// Rilascia il lock
	pthread_mutex_unlock(buffer->buffer_mutex);
	
	return BUFFER_OK;
}

void* getBloccante(buffer_t* buffer) {
	// Usa un timer per evitare attese infinite
	struct timespec timer;
	struct timeval now;
	
	// Ottieni il lock sulla risorsa
	pthread_mutex_lock(buffer->buffer_mutex);
		while(buffer->K==0) {
			// Inizializza il timer
			gettimeofday(&now, NULL);
			timer.tv_sec = now.tv_sec;
			timer.tv_nsec = now.tv_usec * 1000;
			timer.tv_sec += MAX_WAIT;
			
			// Attendi che il buffer non sia vuoto per un tempo max di MAX_WAIT sec
			if (pthread_cond_timedwait(buffer->buffer_non_vuoto_cv, buffer->buffer_mutex, &timer) == ETIMEDOUT) {
				// Superato il tempo massimo ->
				// rilascia il lock, annulla operazione e ritorna un codice d'errore
				pthread_mutex_unlock(buffer->buffer_mutex);
				return WAIT_MSG;
			}
		}
		
		// Esegui operazione
		void* msg = buffer->buffer[buffer->T];
		buffer->T = (buffer->T + 1) % buffer->N;
		buffer->K = buffer->K - 1;

		// Segnala lo "svuotamento" del buffer
		pthread_cond_signal(buffer->buffer_non_pieno_cv);
	// Rilascia il lock
	pthread_mutex_unlock(buffer->buffer_mutex);

	return msg;
}

void* getBloccanteB(buffer_t* buffer, void **output) {
	// Usa un timer per evitare attese infinite
	struct timespec timer;
	struct timeval now;
	
	// Ottieni il lock sulla risorsa
	pthread_mutex_lock(buffer->buffer_mutex);
		while(buffer->K==0) {
			// Inizializza il timer
			gettimeofday(&now, NULL);
			timer.tv_sec = now.tv_sec;
			timer.tv_nsec = now.tv_usec * 1000;
			timer.tv_sec += MAX_WAIT;
			
			// Attendi che il buffer non sia vuoto per un tempo max di MAX_WAIT sec
			if (pthread_cond_timedwait(buffer->buffer_non_vuoto_cv, buffer->buffer_mutex, &timer) == ETIMEDOUT) {
				// Superato il tempo massimo ->
				// rilascia il lock, annulla operazione e ritorna un codice d'errore
				pthread_mutex_unlock(buffer->buffer_mutex);
				return WAIT_MSG;
			}
		}
		
		// Esegui operazione
		*output = buffer->buffer[buffer->T];
		buffer->T = (buffer->T + 1) % buffer->N;
		buffer->K = buffer->K - 1;

		// Segnala lo "svuotamento" del buffer
		pthread_cond_signal(buffer->buffer_non_pieno_cv);
	// Rilascia il lock
	pthread_mutex_unlock(buffer->buffer_mutex);

	return BUFFER_OK;
}

buffer_t* initVuoto(int capacity) {
	buffer_t *buffer = (buffer_t *)malloc(sizeof(buffer_t));
	buffer->N = capacity;
	buffer->D = 0;
	buffer->T = 0;
	buffer->K = 0;

	int i;
	for (i=0; i<buffer->N; i++)
		buffer->buffer[i] = BUFFER_VUOTO;		// Valore di default per celle del buffer mai scritte

	buffer->buffer_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(buffer->buffer_mutex, NULL);

	buffer->buffer_non_pieno_cv = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	buffer->buffer_non_vuoto_cv = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(buffer->buffer_non_pieno_cv, NULL);
	pthread_cond_init(buffer->buffer_non_vuoto_cv, NULL);
	
	return buffer;
}

buffer_t* initPieno(int capacity) {
	buffer_t *buffer = (buffer_t *)malloc(sizeof(buffer_t));
	buffer->N = capacity;
	buffer->D = 0;
	buffer->T = 0;
	buffer->K = capacity;

	int i;
	for (i=0; i<buffer->N; i++)
		buffer->buffer[i] = DEF_MSG;		// Valore di default per celle del buffer mai scritte

	buffer->buffer_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(buffer->buffer_mutex, NULL);

	buffer->buffer_non_pieno_cv = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	buffer->buffer_non_vuoto_cv = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_cond_init(buffer->buffer_non_pieno_cv, NULL);
	pthread_cond_init(buffer->buffer_non_vuoto_cv, NULL);
	
	return buffer;
}

void destroyBuffer(buffer_t* buffer) {
	free(buffer->buffer);
	pthread_mutex_destroy(buffer->buffer_mutex);
	free(buffer->buffer_mutex);
	pthread_cond_destroy(buffer->buffer_non_vuoto_cv);
	free(buffer->buffer_non_vuoto_cv);
	pthread_cond_destroy(buffer->buffer_non_pieno_cv);
	free(buffer->buffer_non_pieno_cv);
}
