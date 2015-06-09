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

TWEET *newTweet(				\
		char   *text,           \
		char   *userName,       \
		char   *coords,         \
		char   *language,       \
		int    favoriteCount,   \
		int    retweetCount,    \
		long   viewsCount       \
){
	TWEET *tw;
	tw = malloc(sizeof(TWEET));

	tw->text = malloc((strlen(text)) + 2);
	tw->userName = malloc((strlen(userName)) + 2);
	tw->coords = malloc((strlen(coords)) + 2);
	tw->language = malloc((strlen(language)) + 2);

	strcpy(tw->text, text);
	strcpy(tw->userName, userName);
	strcpy(tw->coords, coords);
	strcpy(tw->language, language);

	tw->text[strlen(text)] = END_FIELD;
	tw->text[strlen(text)+1] = '\0';
	tw->userName[strlen(userName)] = END_FIELD;
	tw->userName[strlen(userName)+1] = '\0';
	tw->coords[strlen(coords)] = END_FIELD;
	tw->coords[strlen(coords)+1] = '\0';
	tw->language[strlen(language)] = END_FIELD;
	tw->language[strlen(language)+1] = '\0';
	
	tw->favoriteCount = favoriteCount;
	tw->retweetCount = retweetCount;
	tw->viewsCount = viewsCount;

	return tw;
}

char *readField(FILE *stream) {
	char *buffer = NULL;
	char character;
	int counter = 0;

	do {
		character = fgetc(stream);
		buffer = (char *) realloc(buffer, sizeof(char)
				* (counter+1));
		buffer[counter++] = character;
	} while (character != END_FIELD && character != '\n');
	buffer[counter-1] = '\0';

	return buffer;
}

TWEET *readTweet(char *filename, long offset){
	int i, size;
	FILE *dataFile;
	char buff;

	TWEET *tw;

	tw = malloc(sizeof(TWEET));

	dataFile = fopen(filename, "r");

	fseek(dataFile, offset, SEEK_SET);
	fread(&size, sizeof(int), 1, dataFile);

	void *dataRaw;
	dataRaw = malloc(size);

	fread(dataRaw, size, 1, dataFile);

	FILE *mem;
	mem = fmemopen(dataRaw, size, "r");

	tw->text = readField(mem);
	tw->userName = readField(mem);
	tw->coords = readField(mem);
	tw->language = readField(mem);
	fread(&(tw->favoriteCount), sizeof(int), 1, mem); 
	fread(&(tw->retweetCount), sizeof(int), 1, mem); 
	fread(&(tw->viewsCount), sizeof(long), 1, mem);

	return tw; 
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