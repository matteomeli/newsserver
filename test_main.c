/*
 *  test_main.c
 *
 *  Created by Matteo Meli on 2010-05-13.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include <stdio.h>
#include <CUnit/Basic.h>
#include "test_buffer.h"

int main (int argc, char const *argv[])
{
	// Dichiara le suite per i test
	CU_pSuite buffer_ts = NULL;
	
	// Inizializza il registro dei test unitari
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();
		
	// Inizializza le suite
	// Suite per buffer
	buffer_ts = CU_add_suite("Eseguendo i test per buffer...", NULL, NULL);
	
	if (buffer_ts == NULL) {
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	// Aggiungo i test alle suites:
	// Tests per buffer
	if (NULL == CU_add_test(buffer_ts, "Inserimento unitaria in buffer vuoto (C=1)", test_inserisce_un_elemento) ||
	    NULL == CU_add_test(buffer_ts, "Estrazione unitaria da buffer pieno (C=1)", test_estrae_un_elemento) ||
	    NULL == CU_add_test(buffer_ts, "Ins/Ext unitaria da buffer pieno (C=1)", test_inserisce_estrae) ||
	    NULL == CU_add_test(buffer_ts, "Ins/Ext multipli da buffer vuoto/pieno (C=1)", test_inserisce_estrae_elementi) ||
	    NULL == CU_add_test(buffer_ts, "Inserimenti multipli in vari scenari da buffer vuoto (C>1)", test_inserisce_elementi) ||
	    NULL == CU_add_test(buffer_ts, "Estrazioni multiple in vari scenari da buffer pieno (C>1)", test_inserisce_elementi) ||
	    NULL == CU_add_test(buffer_ts, "Inserimento con attesa in buffer vuoto (C=1)", test_inserimento_con_attesa) ||
	    NULL == CU_add_test(buffer_ts, "Estrazione con attesa da buffer pieno (C=1)", test_estrazione_con_attesa) ||
	    NULL == CU_add_test(buffer_ts, "Ins/Ext concorrenti (C=1)", test_inserisce_estrae_concorrente_unitario) ||
	    NULL == CU_add_test(buffer_ts, "Ins/Ext concorrenti (C>1)", test_inserisce_estrae_concorrente)) {
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	// Esegui i test
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}