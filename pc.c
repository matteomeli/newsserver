/*
 *  pc.c
 *
 *  Created by Matteo Meli on 2010-05-15.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "pc.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

int sendString(int socket, const char *str) {
	if (write(socket, str, strlen(str))<0) {
		printf("errore %s durante la write\n", strerror(errno));
		return COMM_ERROR;
	}
	return 1;
}

int receiveString(int socket, char *str, int size) {
	int len = 0;
	if ((len = read(socket, str, size))<0) {
		printf("errore %s durante la read\n", strerror(errno));
		return COMM_ERROR;
	}
	str[len] = '\0';
	return len;
}

