#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define FNAME "tweettest"

int main() {
	int foundOccurences, i;
	char * str0 = calloc(sizeof(char), 14);
	char * str1 = calloc(sizeof(char), 14);
	char * str2 = calloc(sizeof(char), 14);
	char * str3 = calloc(sizeof(char), 14);
	
	
	TWEET * tt = newTweet("texto",
		"usuario",
		"4 8 15 S 16 23 42 O",
		"PT-BR",
		10,
		15,
		200);
	if (tt != NULL) { printf("\nTweet criado\n\n"); }
	/*
	printTweet(tt);
	
	writeTweet(FNAME, tt);
	printf("\nTweet escrito *\n\n");
	
	free(tt);
	printf("\nTweet liberado *\n\n");
	
	tt = readTweet(FNAME, sizeof(long));
	if (tt != NULL) { printf("\nTweet lido\n\n"); }
	
	printTweet(tt);
	
	for (i = 0; i < 50; i++){
		
		sprintf(str0, "%d", rand());
		sprintf(str1, "%d", (i%7)*10);
		sprintf(str2, "%d", rand());
		sprintf(str3, "%d", (i%3));
		
		tt = newTweet(
		str0,
		str1,
		str2,
		str3,
		rand()%30,
		rand()%30,
		rand()%200);
	}
	*/
	return 0;
}