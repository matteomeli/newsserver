/*
 *  fixtures_buffer.c
 *
 *  Created by Matteo Meli on 2010-05-13.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "fixtures_buffer.h"
#include "buffer.h"

void init_param(param_t* param, int id, int wait, buffer_t* buffer) {
	param->id = id;
	param->wait = wait;
	param->buffer = buffer;
}

void destroy_param(param_t* param) {
	free(param);
}

void* fixture_inserisci_un_elemento(void* args) {
	int *result;
	
	// Produci un singolo inserimento nel buffer
	buffer_t* buffer = (buffer_t*)args;
	result = (int *)putBloccante(buffer, TEST_MSG);
	
	// Ritorna
	if (result == BUFFER_OK)
		pthread_exit(TEST_OK);
	else
		pthread_exit(TEST_ERR);
}

void* fixture_estrai_un_elemento(void* args) {
	int *result;
	
	// Effettua una singola estrazione dal buffer
	buffer_t* buffer = (buffer_t*)args;
	result = (int *)getBloccante(buffer);
	
	// Ritorna
	if (result == WAIT_ERR)
		pthread_exit(TEST_ERR);
	else
		pthread_exit((void *)result);
}

void* fixture_aspetta_inserisci_un_elemento(void* args) {
	int *result;
	
	// Ottieni una struttura param_t
	param_t* param = (param_t*)args;
	
	// Aspetta un segnale per partire
	while (param->wait)
		;//printf("Waiting\n");

	//printf("Go!\n");
	// Produci un singolo inserimento nel buffer		
	result = (int *)putBloccante(param->buffer, TEST_MSG);
	
	// Ritorna
	if (result == BUFFER_OK)
		pthread_exit(TEST_OK);
	else
		pthread_exit(TEST_ERR);
}


void* fixture_aspetta_estrai_un_elemento(void* args) {
	int *result;
	
	// Ottieni una struttura param_t
	param_t* param = (param_t*)args;
	
	// Aspetta un segnale per partire
	while (param->wait)
		;//printf("Waiting\n");
	
	//printf("Go!\n");
	// Effettua una singola estrazione dal buffer
	result = (int *)getBloccante(param->buffer);
	
	// Ritorna
	if (result == WAIT_ERR)
		pthread_exit(TEST_ERR);
	else
		pthread_exit((void *)result);
}