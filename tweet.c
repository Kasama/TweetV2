#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(int))	// Size of the data before the first tweet
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

static void destoryTweet (TWEET **tweet){
	if(tweet == NULL || *tweet == NULL)
		return;

	free((*tweet)->text);
	free((*tweet)->userName);
	free((*tweet)->coords);
	free((*tweet)->language);
	
	free(*tweet);
	tweet == NULL;
}
