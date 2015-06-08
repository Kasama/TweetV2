#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(long))	// Size of the data before the first tweet

struct tweet{

	char  *text;
	char  *userName;
	char  *coords;
	char  *language;
	int   favoriteCount;
	int   retweetCount;
	long  viewsCount;

};

void writeTweet(char *filename, TWEET *tweet){
	return;
}

TWEET *readTweet(char *filename, long offset){
	return NULL;
}

long *findOffsetByUser(char *filename, char *username, long *foundOccurences){
	*foundOccurences = 0;
	FILE *f = fopen(filename, "r");
	if (f == NULL) { printf("Erro na leitura do arquivo.\n"); return NULL; }
	
	TWEET *tt = NULL;
	long *listOffset;
	long offset = HEADER;		// Offset of the last begin of tweet
	long nextTweet;				// Offset from the last begin of tweet until the next begin of tweet
	
	fseek(f, offset, SEEK_SET);
	while(fread(&nextTweet, sizeof(long), 1, f)) {
		if (nextTweet > 0){
			tt = readTweet(filename, offset);
			if (!strcmp(username, tt->userName)) {
				*foundOccurences++;
				listOffset = realloc(listOffset, *foundOccurences);
				listOffset[*foundOccurences-1] = offset;
			}
			free(tt);
		}
		
		offset += abs(nextTweet) + sizeof(long);
	}
	
	fclose(f);
	return listOffset;
}

long *findOffsetByFavoriteCount(char *filename, int favoriteCount, long *foundOccurences){
	return 0;
}

long *findOffsetByLanguage(char *filename, char* language, long *foundOccurences){
	return 0;
}

int removeTweet(char *filename, long offset){
	return 0;
}

void printTweet(TWEET *tweet){
	return;
}
