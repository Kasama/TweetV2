#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(long))	// Size of the data before the first tweet
#define minTweetSize  (4 + 2*sizeof(int) + sizeof(long))

struct tweet{

	char  *text;
	char  *userName;
	char  *coords;
	char  *language;
	int   favoriteCount;
	int   retweetCount;
	long  viewsCount;

};

static long bestFit(char* filename, long tweetSize, long *minPreviusOffset){

	long offset, stackPos, currentPreviusOffset;
	int currentDifference, minDifference, blockSize;
	*minPreviusOffset = 0;

	FILE *arq = fopen(filename, "r");
	if(arq == NULL)
		return -1;

	if(fread(&stackPos, sizeof(long), 1, arq) <= 0) goto BESTFIT_ERROR;

	if(stackPos <= 0){
		if(arq !=NULL)
			fclose(arq);
		return 0;
	}

	if(fseek(arq, stackPos, SEEK_SET) != 0)		goto BESTFIT_ERROR;
	if(fread(&minDifference, sizeof(int), 1, arq) <= 0 )	goto BESTFIT_ERROR;
	minDifference *= -1;
	currentPreviusOffset = stackPos;
	if(fread(&stackPos, sizeof(long), 1, arq) <= 0) goto BESTFIT_ERROR;

	while(stackPos != -1 && minDifference >= 0){
		if(fseek(arq, stackPos, SEEK_SET) != 0) goto BESTFIT_ERROR;
		if(fread(&blockSize, sizeof(int), 1, arq) <= 0) goto BESTFIT_ERROR;
		blockSize *= -1;
		currentDifference = blockSize - tweetSize;

		if(currentDifference >= 0 && currentDifference < minDifference){
			minDifference = currentDifference;
			*minPreviusOffset = currentPreviusOffset;
		}
		currentPreviusOffset = stackPos;	
		if(fread(&stackPos, sizeof(long), 1, arq) <= 0) goto BESTFIT_ERROR;
	}

BESTFIT_ERROR:
	if(arq != NULL)
		fclose(arq);
	return offset;	
}

static long tweetSize(TWEET* tweet){
	long sum = 0;
	sum += strlen(tweet->text);
	sum += strlen(tweet->userName);
	sum += strlen(tweet->coords);
	sum += strlen(tweet->language);
	sum += 2*sizeof(int) + sizeof(long);
	
	return sum;
}

static void flushTweet(char* filename, long byteOffset, long fieldsize, TWEET *tweet){

	FILE *arq = NULL;
	if(byteOffset == 0){
		arq = fopen(filename, "a");
		if(arq == NULL) return;
	}else{
		arq = fopen(filename, "w+");
		if(arq == NULL) return;
		if(fseek(arq, byteOffset, SEEK_SET) != 0)	goto FLUSHTWEET_EXIT;
	}
	if(fwrite(&fieldsize, sizeof(int), 1, arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->text, sizeof(int), 1, arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->userName, sizeof(int), 1, arq) <= 0)	goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->coords, sizeof(int), 1, arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->language, sizeof(int), 1, arq) <= 0)	goto FLUSHTWEET_EXIT;

FLUSHTWEET_EXIT:
	if(arq != NULL)
		fclose(arq);
}

void writeTweet(char *filename, TWEET *tweet){
	if(filename == NULL || tweet  == NULL) return;

	long tweetsize = tweetSize(tweet);
	long previusOffset, previusOffsetValue;
	long byteOffset  = bestFit(filename, tweetsize, &previusOffset);
	
	if(filename == NULL || tweet == NULL) return;

	FILE *arq = fopen(filename, "w+");
	if(arq == NULL) return;

	int fieldSize;
	if(fread(&fieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT;
	fieldSize *= -1;
	
	long newFieldSize = fieldSize - (tweetsize + sizeof(int));
	if(byteOffset == 0){
		flushTweet(filename, byteOffset, tweetsize, tweet);
	}
	else if(newFieldSize < minTweetSize){
		if(fseek(arq, byteOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT;
		if(fread(&previusOffsetValue, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT;
		if(fseek(arq, previusOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT;
		if(fwrite(&previusOffsetValue, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT;
		flushTweet(filename, byteOffset, fieldSize, tweet);
	}else{
		newFieldSize *= -1;			
		if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT;
		if(fwrite(&newFieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT;
		byteOffset += sizeof(int) + newFieldSize;
		flushTweet(filename, byteOffset, tweetsize, tweet);
	}

WRITETWEET_EXIT:
	if(arq != NULL)
		fclose(arq);
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
