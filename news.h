/*
 *  news.h
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef NEWS_H
#define NEWS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct news_t {
	char *sender;
	char *topic;
	char *message;
	int serialnumber;
} news_t;

news_t* makeNews(char *sender, char *topic, char *message, int serialnumber);

char* news2Message(news_t *news);

void destroyNews(news_t *news);

#endif /* end of include guard: NEWS_H */

