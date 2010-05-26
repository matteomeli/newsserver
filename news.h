/*
 *  news.h
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#ifndef NEWS_H
#define NEWS_H

#include <stdlib.h>
#include <string.h>

typedef struct news_t {
	int id;
	char *topic;
	char *message;
} news_t;

news_t* makeNews(char *topic, char *message, int id);

#endif /* end of include guard: NEWS_H */

