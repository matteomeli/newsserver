/*
 *  test_buffer.c
 *
 *  Created by Matteo Meli on 2010-05-13.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "test_buffer.h"
#include "fixtures_buffer.h"
#include "buffer.h"

void test_inserisce_un_elemento(void) {
	int result = 0;
	int expected_msgs = 1;
	int expected_index = 0;
	
	// Crea un buffer vuoto per il test
	buffer_t* buffer_vuoto = (buffer_t*)malloc(sizeof(buffer_t));
	// Inizializza il buffer con capacità 1
	init_vuoto(buffer_vuoto, 1);
	
	// Sollecita il buffer con un inserimento
	pthread_t p;
	pthread_create(&p, NULL, &fixture_inserisci_un_elemento, buffer_vuoto);
	pthread_join(p, (void*)&result);
	
	// Verifica l'inserimento unitario
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL((int)buffer_vuoto->buffer[buffer_vuoto->T], (int)TEST_MSG);
	CU_ASSERT_EQUAL(result, (int)TEST_OK);
	
	// Sollecita il buffer con un altro inserimento
	pthread_t r;
	pthread_create(&r, NULL, &fixture_inserisci_un_elemento, buffer_vuoto);
	pthread_join(r, (void*)&result);
	
	// Verifica che il secondo inserimento non va a buon fine
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_vuoto->D, expected_index);
	CU_ASSERT_EQUAL((int)buffer_vuoto->buffer[buffer_vuoto->T], (int)TEST_MSG);
	CU_ASSERT_EQUAL(result, (int)TEST_ERR);
	
	destroy_buffer(buffer_vuoto);
}

void test_estrae_un_elemento(void) {
	int result = 0;
	int expected_index = 0;
	int expected_msgs = 0;
	
	// Crea un buffer pieno per il test
	buffer_t* buffer_pieno = (buffer_t*)malloc(sizeof(buffer_t));
	// Inizializza il buffer con capacità 1
	init_pieno(buffer_pieno, 1);
	
	// Sollecita il buffer con una estrazione
	pthread_t p;
	pthread_create(&p, NULL, &fixture_estrai_un_elemento, buffer_pieno);
	pthread_join(p, (void *)&result);
	
	// Verifica l'estrazione
	CU_ASSERT_EQUAL(buffer_pieno->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_pieno->T, expected_index);
	CU_ASSERT_EQUAL(result, (int)DEF_MSG);
	
	// Prova ad effettuare un'altra estrazione
	// (non dovrebbe andare a buon fine)
	pthread_t r;
	pthread_create(&r, NULL, &fixture_estrai_un_elemento, buffer_pieno);
	pthread_join(r, (void *)&result);
	
	// Verifica il fallimento
	CU_ASSERT_EQUAL(buffer_pieno->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_pieno->T, expected_index);
	CU_ASSERT_EQUAL(buffer_pieno->D, expected_index);
	CU_ASSERT_EQUAL(result, (int)TEST_ERR);
	
	destroy_buffer(buffer_pieno);
}

void test_inserisce_estrae(void) {
	int result_ins = 0;
	int result_est = 0;
	int expected_index = 0;
	int expected_msgs = 0;
	
	// Crea un buffer vuoto per il test
	buffer_t* buffer_vuoto = (buffer_t*)malloc(sizeof(buffer_t));
	// Inizializza il buffer con capacità 1
	init_vuoto(buffer_vuoto, 1);
	
	// Sollecita il buffer con inserimento ed estrazione concorrenti
	pthread_t p, c;
	pthread_create(&p, NULL, &fixture_inserisci_un_elemento, buffer_vuoto);
	pthread_create(&c, NULL, &fixture_estrai_un_elemento, buffer_vuoto);
	pthread_join(p, (void *)&result_ins);
	pthread_join(c, (void *)&result_est);
	
	// Verifica il successo delle due operazioni
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_vuoto->T, expected_index);
	CU_ASSERT_EQUAL(buffer_vuoto->D, expected_index);
	CU_ASSERT_EQUAL(result_ins, (int)TEST_OK);
	CU_ASSERT_EQUAL(result_est, (int)TEST_MSG);
	
	// Ora sollecita il buffer con estrazione e inserimento concorrenti
	pthread_t pp, cc;
	pthread_create(&cc, NULL, &fixture_estrai_un_elemento, buffer_vuoto);
	pthread_create(&pp, NULL, &fixture_inserisci_un_elemento, buffer_vuoto);
	pthread_join(cc, (void *)&result_est);
	pthread_join(pp, (void *)&result_ins);
	
	// Verifica il successo dell'operazione
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_vuoto->T, expected_index);
	CU_ASSERT_EQUAL(buffer_vuoto->D, expected_index);
	CU_ASSERT_EQUAL(result_ins, (int)TEST_OK);
	CU_ASSERT_EQUAL(result_est, (int)TEST_MSG);
	
	destroy_buffer(buffer_vuoto);
}

void test_inserisce_estrae_elementi(void) {
	const int NUM_INS = 10;
	int result_ins[NUM_INS];
	int result_est[NUM_INS];
	int expected_puts = 1;
	int expected_gets = 1;
	int expected_msgs = 1;
	int expected_index = 0;
	int i;
	
	// Crea un buffer vuoto per il test
	buffer_t* buffer_vuoto = (buffer_t*)malloc(sizeof(buffer_t));
	// Inizializza il buffer con capacità 1
	init_vuoto(buffer_vuoto, 1);
	
	// Sollecita il buffer con inserimenti multipli concorrenti
	pthread_t cc[NUM_INS];
	for (i=0; i<NUM_INS; i++)
		pthread_create(&cc[i], NULL, &fixture_inserisci_un_elemento, buffer_vuoto);
	for (i=0; i<NUM_INS; i++)
		pthread_join(cc[i], (void *)&result_ins[i]);
	
	// Verifica che un solo inserimento è andato a buon fine
	int success_cnt = 0;
	int failure_cnt = 0;
	for (i=0; i<NUM_INS; i++)
		if (result_ins[i]!=(int)TEST_ERR)
			success_cnt++;
		else
			failure_cnt++;
	
	CU_ASSERT_EQUAL(success_cnt, expected_puts);
	CU_ASSERT_EQUAL(failure_cnt, NUM_INS-success_cnt);
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_vuoto->D, expected_index);
	CU_ASSERT_EQUAL((int)buffer_vuoto->buffer[buffer_vuoto->T], (int)TEST_MSG);
	
	// Sollecita il buffer, ora pieno, con consumazioni multiple concorrenti
	pthread_t rr[NUM_INS];
	for (i=0; i<NUM_INS; i++)
		pthread_create(&rr[i], NULL, &fixture_estrai_un_elemento, buffer_vuoto);
	for (i=0; i<NUM_INS; i++)
		pthread_join(cc[i], (void *)&result_est[i]);
		
	// Verifica che una sola estrazione è andato a buon fine
	success_cnt = 0;
	failure_cnt = 0;
	for (i=0; i<NUM_INS; i++)
		if (result_est[i]==(int)TEST_MSG)
			success_cnt++;
		else
			failure_cnt++;
			
	CU_ASSERT_EQUAL(success_cnt, expected_gets);
	CU_ASSERT_EQUAL(failure_cnt, NUM_INS-success_cnt);
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs-1);
	CU_ASSERT_EQUAL(buffer_vuoto->T, expected_index);
	
	destroy_buffer(buffer_vuoto);
}

void test_inserisce_elementi(void) {
	const int DEF_CAP = 10;
	const int NUM_INS = 10;
	int result_ins[NUM_INS];
	int i;
	
	// Crea tre buffer vuoti utili per il test
	buffer_t* buffer1 = (buffer_t*)malloc(sizeof(buffer_t));
	buffer_t* buffer2 = (buffer_t*)malloc(sizeof(buffer_t));
	buffer_t* buffer3 = (buffer_t*)malloc(sizeof(buffer_t));
	init_vuoto(buffer1, DEF_CAP+5);
	init_vuoto(buffer2, DEF_CAP-5);
	init_pieno(buffer3, DEF_CAP);
	
	// Sollecita il buffer1 uno con inserimenti multipli concorrenti
	pthread_t pp[NUM_INS];
	for (i=0; i<NUM_INS; i++)
		pthread_create(&pp[i], NULL, &fixture_inserisci_un_elemento, buffer1);
	for (i=0; i<NUM_INS; i++)
		pthread_join(pp[i], (void *)&result_ins[i]);
	
	// Verifica che tutti gli inserimenti hanno successo
	int success_cnt = 0;
	int failure_cnt = 0;
	for (i=0; i<NUM_INS; i++)
		if (result_ins[i]!=(int)TEST_ERR)
			success_cnt++;
		else
			failure_cnt++;
	
	CU_ASSERT_EQUAL(success_cnt, NUM_INS);
	CU_ASSERT_EQUAL(failure_cnt, 0);
	CU_ASSERT_EQUAL(buffer1->K, NUM_INS);
	CU_ASSERT_EQUAL(buffer1->D, NUM_INS);
	for (i=0; i<NUM_INS; i++)
		CU_ASSERT_EQUAL((int)buffer1->buffer[i], (int)TEST_MSG);
		
	// Sollecita il buffer2 uno con inserimenti multipli concorrenti
	pthread_t pp2[NUM_INS];
	for (i=0; i<NUM_INS; i++)
		pthread_create(&pp2[i], NULL, &fixture_inserisci_un_elemento, buffer2);
	for (i=0; i<NUM_INS; i++)
		pthread_join(pp2[i], (void *)&result_ins[i]);
	
	// Verifica che NUM_INS/2 inserimenti hanno successo
	success_cnt = 0;
	failure_cnt = 0;
	for (i=0; i<NUM_INS; i++)
		if (result_ins[i]!=(int)TEST_ERR)
			success_cnt++;
		else
			failure_cnt++;
			
	CU_ASSERT_EQUAL(success_cnt, NUM_INS/2);
	CU_ASSERT_EQUAL(failure_cnt, NUM_INS/2);
	CU_ASSERT_EQUAL(buffer2->K, NUM_INS/2);
	CU_ASSERT_EQUAL(buffer2->D, 0);
	for (i=0; i<NUM_INS/2; i++)
			CU_ASSERT_EQUAL((int)buffer2->buffer[i], (int)TEST_MSG);
			
	// Sollecita il buffer3 uno con inserimenti multipli concorrenti
	pthread_t pp3[NUM_INS];
	for (i=0; i<NUM_INS; i++)
		pthread_create(&pp3[i], NULL, &fixture_inserisci_un_elemento, buffer3);
	for (i=0; i<NUM_INS; i++)
		pthread_join(pp3[i], (void *)&result_ins[i]);
	
	// Verifica che 0 inserimenti hanno successo
	success_cnt = 0;
	failure_cnt = 0;
	for (i=0; i<NUM_INS; i++)
		if (result_ins[i]!=(int)TEST_ERR)
			success_cnt++;
		else
			failure_cnt++;
			
	CU_ASSERT_EQUAL(success_cnt, 0);
	CU_ASSERT_EQUAL(failure_cnt, NUM_INS);
	CU_ASSERT_EQUAL(buffer3->K, DEF_CAP);
	CU_ASSERT_EQUAL(buffer3->D, 0);
	for (i=0; i<NUM_INS/2; i++)
			CU_ASSERT_EQUAL((int)buffer3->buffer[i], (int)DEF_MSG);
		
	destroy_buffer(buffer1);
	destroy_buffer(buffer2);
	destroy_buffer(buffer3);
}

void test_estrae_elementi(void) {
	const int DEF_CAP = 10;
	const int NUM_EST = 10;
	int result_est[NUM_EST];
	int i;
	
	// Crea 2 buffer pieni per il test
	buffer_t* buffer1 = (buffer_t*)malloc(sizeof(buffer_t));
	buffer_t* buffer2 = (buffer_t*)malloc(sizeof(buffer_t));
	buffer_t* buffer3 = (buffer_t*)malloc(sizeof(buffer_t));
	init_pieno(buffer1, DEF_CAP+5);
	init_pieno(buffer2, DEF_CAP-5);
	init_vuoto(buffer3, DEF_CAP);
	
	// Sollecita il buffer1 con estrazioni multiple concorrenti
	pthread_t cc1[NUM_EST];
	for (i=0; i<NUM_EST; i++)
		pthread_create(&cc1[i], NULL, &fixture_estrai_un_elemento, buffer1);
	for (i=0; i<NUM_EST; i++)
		pthread_join(cc1[i], (void *)&result_est[i]);
	
	// Verifica che tutte le estrazioni hanno successo
	int success_cnt = 0;
	int failure_cnt = 0;
	for (i=0; i<NUM_EST; i++)
		if (result_est[i]==(int)TEST_MSG)
			success_cnt++;
		else
			failure_cnt++;

	CU_ASSERT_EQUAL(success_cnt, NUM_EST);
	CU_ASSERT_EQUAL(failure_cnt, 0);
	CU_ASSERT_EQUAL(buffer1->K, DEF_CAP-NUM_EST);
	CU_ASSERT_EQUAL(buffer1->T, NUM_EST);
	
	// Sollecita il buffer2 con estrazioni multiple concorrenti
	pthread_t cc2[NUM_EST];
	for (i=0; i<NUM_EST; i++)
		pthread_create(&cc2[i], NULL, &fixture_estrai_un_elemento, buffer2);
	for (i=0; i<NUM_EST; i++)
		pthread_join(cc2[i], (void *)&result_est[i]);
	
	// Verifica che NUM_EST estrazioni hanno successo
	success_cnt = 0;
	failure_cnt = 0;
	for (i=0; i<NUM_EST; i++)
		if (result_est[i]==(int)TEST_MSG)
			success_cnt++;
		else
			failure_cnt++;

	CU_ASSERT_EQUAL(success_cnt, NUM_EST/2);
	CU_ASSERT_EQUAL(failure_cnt, NUM_EST/2);
	CU_ASSERT_EQUAL(buffer1->K, 0);
	CU_ASSERT_EQUAL(buffer1->T, 0);
	
	// Sollecita il buffer3 con estrazioni multiple concorrenti
	pthread_t cc3[NUM_EST];
	for (i=0; i<NUM_EST; i++)
		pthread_create(&cc3[i], NULL, &fixture_estrai_un_elemento, buffer3);
	for (i=0; i<NUM_EST; i++)
		pthread_join(cc3[i], (void *)&result_est[i]);
	
	// Verifica che 0 estrazioni hanno successo
	success_cnt = 0;
	failure_cnt = 0;
	for (i=0; i<NUM_EST; i++)
		if (result_est[i]==(int)TEST_MSG)
			success_cnt++;
		else
			failure_cnt++;

	CU_ASSERT_EQUAL(success_cnt, 0);
	CU_ASSERT_EQUAL(failure_cnt, NUM_EST);
	CU_ASSERT_EQUAL(buffer1->K, 0);
	CU_ASSERT_EQUAL(buffer1->T, 0);
	
	destroy_buffer(buffer1);
	destroy_buffer(buffer2);
	destroy_buffer(buffer3);
}

void test_inserimento_con_attesa(void) {
	int result = 0;
	int expected_msgs = 1;
	int expected_index = 0;
	
	// Crea un buffer vuoto per il test
	buffer_t* buffer_vuoto = (buffer_t*)malloc(sizeof(buffer_t));
	// Inizializza il buffer con capacità 1
	init_vuoto(buffer_vuoto, 1);
	
	// Inizializza struttura param_t con wait = 1
	param_t* param = (param_t*)malloc(sizeof(buffer_t));
	init_param(param, 1, 1, buffer_vuoto);
	
	// Sollecita il buffer con un inserimento
	pthread_t p;
	pthread_create(&p, NULL, &fixture_aspetta_inserisci_un_elemento, param);
	
	// Segnala al "waiting" thread di partire
	sleep(5);
	param->wait = 0;
	
	pthread_join(p, (void*)&result);
	
	// Verifica l'inserimento unitario
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL((int)buffer_vuoto->buffer[buffer_vuoto->T], (int)TEST_MSG);
	CU_ASSERT_EQUAL(result, (int)TEST_OK);
	
	// Sollecita il buffer con un altro inserimento
	pthread_t r;
	pthread_create(&r, NULL, &fixture_inserisci_un_elemento, buffer_vuoto);
	pthread_join(r, (void*)&result);
	
	// Verifica che il secondo inserimento non va a buon fine
	CU_ASSERT_EQUAL(buffer_vuoto->K, expected_msgs);
	CU_ASSERT_EQUAL(buffer_vuoto->D, expected_index);
	CU_ASSERT_EQUAL((int)buffer_vuoto->buffer[buffer_vuoto->T], (int)TEST_MSG);
	CU_ASSERT_EQUAL(result, (int)TEST_ERR);
	
	destroy_buffer(buffer_vuoto);
}

void test_estrazione_con_attesa(void) {
	// TODO
}

void test_inserisce_estrae_concorrente_unitario(void) {
	// TODO
}

void test_inserisce_estrae_concorrente(void) {
	// TODO
}