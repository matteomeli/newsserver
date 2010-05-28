/*
 *  test_list.c
 *
 *  Created by Matteo Meli on 2010-05-24.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include <stdio.h>
#include "list.h"

int main() {
	list_t *list = NULL;
	list = allocList();
	
	addElement(list, (void *)1);
	addElement(list, (void *)2);
	addElement(list, (void *)3);
	
	printf("list count: %d\n", count(list));
	
	list_t *iterator = list->next;
	printf("lista:\n");
	while (iterator) {
		printf("%d ", (int)iterator->data);
		iterator = iterator->next;
	}
	printf("\n");
	
	removeElement(list, (void *)1);
	
	iterator = list->next;
	printf("lista:\n");
	while (iterator) {
		printf("%d ", (int)iterator->data);
		iterator = iterator->next;
	}
	printf("\n");
	
	// stampa la lista con l'iteratore
	printf("lista:\n");
	iterator_t *i = createIterator(list);
	while(hasNext(i)) {
		int e = (int)next(i);
		printf("%d ", e);
	}
	printf("\n");
	freeIterator(i);
	
	freeList(list);
	
	return 0;
}