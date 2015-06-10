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
	while(fread(&nextTweet, 
		sizeof(long), 1, f)) {
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
	if(filename == NULL) return 0;

	// tries to open the file and check if it was successful
	FILE *file = fopen(filename, "r+");
	if(file == NULL) return 0;

	//getting the stack's head
	long stackHead;
	if(fread(&stackHead, HEADER, 1, file) == 0)
		goto fileFunctionError;
	
	//tries to move til the deletion's offset
	if(fseek(file, offset, SEEK_SET) != 0)
		goto fileFunctionError;
	int fieldSize;

	//tries to save the size of the register and moves sizeof(int) forward
	if(fread(&fieldSize, sizeof(int), 1, file) == 0)
		goto fileFunctionError;

	//checks if the register is already removed
	if(fieldSize <= 0) {
		fclose(file);
		return 0;
	}

	//tries to write a long that indicates the position of the last reg removed (stack's head)
	if(fwrite(&stackHead, sizeof(long), 1, file) == 0)
		goto fileFunctionError;

	//tries to update the stack's head in the file's beginning
	if(fseek(file, SEEK_SET, SEEK_SET) != 0)
		goto fileFunctionError;
	if(fwrite(&offset, sizeof(long), 1, file) == 0)
		goto fileFunctionError;

	//tries to back to the register and mark it as removed (negative)
	if(fseek(file, offset, SEEK_SET) != 0)
		goto fileFunctionError;
	fieldSize *= -1;
	if(fwrite(&fieldSize, sizeof(int), 1, file) == 0)
		goto fileFunctionError;

	fclose(file);
	return 1;

fileFunctionError:
	fclose(file);
	return 0;
}

void printTweet(TWEET *tweet){
	if(tweet == NULL) return;

	printf("Tweet: %s\n", 			tweet->text);
	printf("User: %s\n" , 	 		tweet->userName);
	printf("Coordinate: %s\n", 		tweet->coords);
	printf("Language: %s\n",		tweet->language);
	printf("Favorited %d time%s",tweet->viewsCount, (tweet->favoriteCount <= 1)?"\n":"s\n");
	printf("Retweeted %d time%s",tweet->retweetCount, (tweet->retweetCount <= 1)?"\n":"s\n");
	printf("Viewed %d time%s",tweet->viewsCount, (tweet->viewCount <= 1)?"\n":"s\n");
	printf("_________________________________________________");

	return;
}
