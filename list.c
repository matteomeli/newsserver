/*
 *  list.c
 *  Defines an implementation of the "list" interface.
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "list.h"

list_t* allocList() {
	list_t *head = (list_t *)malloc(sizeof(list_t));
	head->data = NULL;
	head->next = NULL;
	return head;
}

void freeList(list_t *list) {
	list_t *curr;
	curr = list;
	while (curr) {
		list_t *next = curr->next;
		free(curr);
		curr = next;
	}
}

void addElement(list_t *list, void *element) {
	list_t *new, *curr;
	new = allocList();
	new->data = element;

	curr = list;
	while (curr->next)
		curr = curr->next;

	curr->next = new;
}

void removeElement(list_t *list, void *element) {
	list_t* curr = list;
	while (curr->next && curr->next->data!=element) {
		curr = curr->next;
	}
	if (curr->next) {
		list_t *match = curr->next;
		curr->next = curr->next->next;
		free(match);
	}
}

iterator_t* createIterator(list_t *list) {
	iterator_t *it = (iterator_t *)malloc(sizeof(iterator_t));
	if (!list->data)
		it->next = list->next;		// La testa è NULL, comincio dal *vero* primo elemento.
	else
		it->next = list;					// Comincio dal primo elemento.
	it->index = 0;
	return it;
}

void freeIterator(iterator_t *it) {
	free(it);
}

int hasNext(iterator_t *it) {
	return (it->next!=NULL);
}

void* next(iterator_t *it) {
	list_t *next = it->next;
	it->next = it->next->next;
	it->index++;
	return next->data;
}

