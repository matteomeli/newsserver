
/*
 * list.h
 *
 *  Created on: 15/dic/2009
 *      Author: matte
 */

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

// Gestione di liste tipate puntatori a void (void *)
// Gli elementi non possono essere NULL, solo la testa

// struttura lista
typedef struct list_t {
	void *data;
	struct list_t *next;
} list_t;

// struttura iteratore
typedef struct iterator_t {
	list_t *next;
	int index;
} iterator_t;

// Alloca una lista
list_t* allocList();

// Dealloca una lista
void freeList(list_t *list);

// Aggiunge in fondo
void addElement(list_t *list, void *element);

// Rimuove un elemento
void removeElement(list_t *list, void *element);

// Conta gli elementi nella lista
int count(list_t *list);

// Crea un iteratore sulla lista
iterator_t* createIterator(list_t *list);

// Dealloca un iteratore
void freeIterator(iterator_t *it);

// scansione finita?
int hasNext(iterator_t *it);

// prossimo elemento, NULL se finiti
void* next(iterator_t *it);

#endif /* LIST_H */
