#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(int))	// Size of the data before the first tweet

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

TWEET *readTweet(char *filename, int offset){
	return NULL;
}

int *findOffsetByUser(char *filename, char *username, int *foundOccurences){
	*foundOccurences = 0;
	FILE *f = fopen(filename, "r");
	if (f == NULL) { printf("Erro na leitura do arquivo.\n"); return NULL; }
	
	TWEET *tt = NULL;
	int *listOffset;
	int offset = HEADER;		// Offset of the last begin of tweet
	int nextTweet;				// Offset from the last begin of tweet until the next begin of tweet
	int sizeText, sizeUsername;	// Size of Text and Username of this tweet
	char *userRead;				// Username read from file
	
	fseek(f, offset, SEEK_SET);
	while(fread(&nextTweet, sizeof(int), 1, f)) {
		if (nextTweet > 0){
			tt = readTweet(filename, offset);
			if (!strcmp(username, tt->userName)) {
				*foundOccurences++;
				listOffset = realloc(listOffset, *foundOccurences);
				listOffset[*foundOccurences-1] = offset;
			}
			free(tt);
		}
		
		offset += abs(nextTweet) + sizeof(int);
	}
	
	fclose(f);
	return listOffset;
}

int *findOffsetByFavoriteCount(char *filename, int favoriteCount, int *foundOccurences){
	return 0;
}

int *findOffsetByLanguage(char *filename, char* language, int *foundOccurences){
	return 0;
}

int removeTweet(char *filename, int offset){
	return 0;
}

void printTweet(TWEET *tweet){
	return;
}