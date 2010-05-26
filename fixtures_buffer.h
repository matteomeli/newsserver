/*
 *  fixtures_buffer.h
 *
 * This file defines some useful thread function to test "buffer".
 *
 *  Created by Matteo Meli on 2010-05-13.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef FIXTURES_BUFFER_H
#define FIXTURES_BUFFER_H

#include "buffer.h"

#define TEST_OK 	(void *)  10
#define TEST_ERR 	(void *) -10
#define TEST_MSG 	(void *)   3

/*
 * Struttura che definisce l'argomento per le fixture "signal waiting" */
typedef struct param_t {
	int id;
	int wait;
	buffer_t* buffer;
} param_t;

/*
 * Inizializza una struttura parametro per le fixture
 */
void init_param(param_t* param, int id, int wait, buffer_t* buffer);

/*
 * Dealloca una struttura parametro
 */
void destroy_param(param_t* param);

/*
 * Definisce un generico inserimento unitario in un buffer.
 */
void* fixture_inserisci_un_elemento(void* args);

/*
 * Definisce una generica estrazione in un buffer.
 */
void* fixture_estrai_un_elemento(void* args);

/*
 * Definisce un inserimento in un buffer (signal waiting) 
 */
void* fixture_aspetta_inserisci_un_elemento(void* args);

/*
 * Definisce una estrazione da un buffer (signal waiting) 
 */
void* fixture_aspetta_estrai_un_elemento(void* args);

#endif /* end of include guard: FIXTURES_BUFFER_H */
