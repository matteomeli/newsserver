/*
 *  pc.h
 *
 *  Created by Matteo Meli on 2010-05-15.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef PC_H
#define PC_H

#define COMM_ERROR	-1

/* Send a string str over a socket  */
int sendString(int socket, const char *str);

/* Receive data from a socket */
int receiveString(int socket, char *str, int size);

#endif /* end of include guard: PC_H*/
