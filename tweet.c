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
	fread(&stackHead, sizeof(long), 1, file);
	
	//moving til the deletion's offset
	fseek(file, offset, SEEK_SET);
	int fieldSize;
	//saves the size of the register and moves sizeof(int) forward
	fread(&fieldSize, sizeof(int), 1, file);
	//checks if the register is already removed
	if(fieldSize <= 0){
		fclose(file);
		return 0;
	}
	//writes a long that indicates the position of the last reg removed (stack's head)
	fwrite(&stackHead, sizeof(long), 1, file);
	//updating the stack's head in the file's beggining
	fseek(file, SEEK_SET, SEEK_SET);
	fwrite(&offset, sizeof(long), 1, file);
	//ver se precisa concatenar o proximo registro

	//goes to the next register and saves it's beggining offset
	fseek(file, offset + fieldSize, SEEK_SET);
	long nextRegisterOffset = ftell(file); //a partir do que tá sendo removido
	//reading the next field Size and checking if it isn't a removed register, in positive case, closes the file and return
	int nextFieldSize;
	fread(&nextFieldSize, sizef(int), 1, file);
		//just backs to the register to be deleted and marks it as removed (negative)
	/*
	if(nextFieldSize > 0) {
		fseek(file, offset, SEEK_SET);
		fieldSize *= -1;
		fwrite(&fieldSize, sizeof(int), 1, file);
		fclose(file);
		return 1;
	}
	*/
	markAsRemoved(file, fieldSize, nextFieldSize);
	return 1;
	//if we get here, it's because the next register is already removed
	//saves the content of the offset of the next register (long)
	long removalLongContent;  //conteúdo do offset (long) do lado do int do registro removido após o registro de offset recebido na função
	fread(&removalLongContent, sizeof(long), 1, file);

	
	long currentOffSet = stackHead;
	long aux = currentOffSet;
	while(currentOffSet != nextRegisterOffset && currentOffSet != -1) {
		fseek(file, currentOffSet + sizeof(int), SEEK_SET);
		aux = currentOffSet;
		fread(&currentOffSet, sizeof(long), 1, file);
	}
	fseek(file, aux + sizeof(int), SEEK_SET);
	fwrite(&removalLongContent, sizeof(long), 1, file);

	markAsRemoved(file, fieldSize, nextFieldSize);
	/*
	fseek(file, offset, SEEK_SET);
	fieldSize *= -1;
	fieldSize += nextFieldSize;
	fwrite(&fieldSize, sizeof(int), 1, file);
	fclose(file);
	*/
	return 1;
}

void markAsRemoved(FILE *file, int fieldSize, int nextFieldSize) {
	fseek(file, offset, SEEK_SET);
	fieldSize *= -1;
	if(nextFieldSize < 0)
		fieldSize += nextFieldSize;
	fwrite(&fieldSize, sizeof(int), 1, file);

	return;
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
