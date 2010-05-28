/*
 *  news.c
 *
 *  Created by Matteo Meli on 2010-05-26.
 *  Copyright (c) 2010 Mattematica. All rights reserved.
 */

#include "news.h"

news_t* makeNews(char *sender, char *topic, char *message, int serialnumber) {
	news_t *news = (news_t *)malloc(sizeof(news_t));
	news->sender = (char *)malloc(strlen(sender)+1);
	news->topic = (char *)malloc(strlen(topic)+1);
	news->message = (char *)malloc(strlen(message)+1);
	strcpy(news->sender, sender);
	strcpy(news->topic, topic);
	strcpy(news->message, message);
	news->serialnumber = serialnumber;
	
	return news;
}

char* news2Message(news_t *news) {
	size_t size = sizeof(strlen(news->sender)+strlen(news->message)+5);
	char *message = (char *)malloc(size);
	sprintf(message, "%d:", news->serialnumber);
	strcat(message, news->sender);
	strcat(message, "~");
	strcat(message, news->message);
	
	return message;	
}

void destroyNews(news_t *news) {
	free(news->sender);
	free(news->topic);
	free(news->message);
	free(news);
}