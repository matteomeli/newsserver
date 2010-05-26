/*
 *  test_buffer.h
 *
 *  Created by Matteo Meli on 2010-05-13.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef TEST_BUFFER_H
#define TEST_BUFFER_H

#include <CUnit/CUnit.h>

/* Inserisce un elemento nel buffer (PUT) */
void test_inserisce_un_elemento(void);

/* Estrae un elemento dal buffer (GET) */
void test_estrae_un_elemento(void);

/* Inserisce ed estrae da un buffer e viceversa (PUT/GET & GET/PUT) */
void test_inserisce_estrae(void);

/* Inserisce e estrae più elementi nel buffer (PUT+ & GET+) */
void test_inserisce_estrae_elementi(void);

/* Inserimenti multipli in vari scenari in buffer con capacità > 1 (PUT+) */
void test_inserisce_elementi(void);

/* Estrazioni multiple in vari scenari in buffer con capacità > 1 (GET+) */
void test_estrae_elementi(void);

/* Inserimento segnalato */
void test_inserimento_con_attesa(void);

/* Estrazione segnalata */
void test_estrazione_con_attesa(void);

/* Inserimenti ed estrazioni concorrenti in un buffer unitario (PUT & GET) */
void test_inserisce_estrae_concorrente_unitario(void);

/* Inserimenti ed estrazioni concorrenti in un buffer con capacità > 1 (PUT & GET) */
void test_inserisce_estrae_concorrente(void);

#endif /* end of include guard: TEST_BUFFER_H */


