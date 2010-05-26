/*
 *  news.c
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "news.h"

news_t* makeNews(char *topic, char *message, int id) {
	news_t *news = (news_t *)malloc(sizeof(news_t));
	news->topic = (char *)malloc(strlen(topic)+1);
	news->message = (char *)malloc(strlen(message)+1);
	strcpy(news->topic, topic);
	strcpy(news->message, message);
	news->id = id;
	
	return news;
}
