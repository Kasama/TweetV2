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

static int removeTweetFromFavoriteIndex(char *filename, TWEET *removedTweet, long offset) {
	char *tableFileName = getFavoriteTableIndexFileName(filename);
	char *listFileName  = getFavoriteListIndexFileName(filename);
	FILE *favoriteTable = fopen(tableFileName, "r+");
	FILE *favoriteList  = fopen(listFileName,  "r+");
	if(favoriteTable == NULL || favoriteList == NULL || removedTweet == NULL) return 0;

	int tableStatus = !UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFLI_EXIT;

	long tableOffset = findIndexOffsetByFavoriteCount(filename, removedTweet->favoriteCount);

	FAVITEM *tableItem;
	if(fseek(favoriteTable, tableOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(tableItem, sizeof(FAVITEM), 1, favoriteTable) <= 0) goto RTFFI_EXIT;

	FAVLISTITEM *current, *previous;
	if(fseek(favoriteList, tableItem.byteOffset, SEEK_SET != 0) goto RTFFI_EXIT;
	if(fread(current, sizeof(FAVLISTITEM), 1, favoriteTable) <= 0)
			goto RTFFI_EXIT;

	if(current.next == -1) {
		if(fseek(favoriteTable, SEEK_END, SEEK_SET) != 0) goto RTFFI_EXIT;
		long sizeOfTableFile = ftell(favoriteTable);
		sizeOfTableFile-= INDEXHEADER;

		FAVITEM *aux = (FAVITEM *) malloc(sizeOfTableFile);
		aux[(tableOffset - INDEXHEADER)/(sizeOfTableFile/sizeof(FAVITEM)] = aux[(sizeOfTableFile/sizeof(FAVITEM))-1];
		realloc(aux, sizeOfTableFile - sizeof(FAVITEM));
		qsort(aux, (sizeOfTableFile/sizeof(FAVITEM)), sizeof(FAVITEM), compareFavoriteItem);
		if(fwrite(aux, sizeOfTableFile), 1, favoriteTable)) goto RTFFI_EXIT;

		rewind(languageTable);
		tableStatus = UPDATED;
		if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFLI_EXIT;
		return 1;
	}

	do {
		previous = current;
		if(fread(current, sizeof(FAVLISTITEM), 1, favoriteTable) <= 0)
			goto RTFFI_EXIT;
		if(fseek(favoriteList, current.fileOffset, SEEK_SET != 0) goto RTFFI_EXIT;
	} while(current.fileOffset != offset || current.next != -1);

	previous.next = current.next;

	rewind(languageTable);
	tableStatus = UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFLI_EXIT;
	return 1;

RTFFI_EXIT:
	fclose(favoriteTable);
	fclose(favoriteList);
	return 0;
}

static int removeTweetFromLanguageIndex(char *filename, TWEET *removedTweet, long offset) {
	char *tableFileName = getLanguageTableIndexFileName(filename);
	char *listFileName  = getLanguageListIndexFileName(filename);
	FILE *languageTable = fopen(tableFileName, "r+");
	FILE *languageList  = fopen(listFileName,  "r+");
	if(languageTable == NULL || languageList == NULL || removedTweet == NULL) return 0;

	int tableStatus = !UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, languageTable) <= 0) goto RTFLI_EXIT;

	long tableOffset = findOffsetByFavoriteCount(filename, removedTweet->language);

	FAVITEM *tableItem;
	if(fseek(languageTable, tableOffset, SEEK_SET) != 0) goto RTFLI_EXIT;
	if(fread(tableItem, sizeof(FAVITEM), 1, languageTable) <= 0) goto RTFLI_EXIT;

	FAVLISTITEM *current, *previous;
	if(fseek(languageList, tableItem.byteOffset, SEEK_SET != 0) goto RTFLI_EXIT;
	if(fread(current, sizeof(FAVLISTITEM), 1, languageTable) <= 0)
			goto RTFLI_EXIT;

	if(current.next == -1) {
		if(fseek(languageTable, SEEK_END, SEEK_SET) != 0) goto RTFFI_EXIT;
		long sizeOfTableFile = ftell(languageTable);
		sizeOfTableFile-= INDEXHEADER;

		FAVITEM *aux = (FAVITEM *) malloc(sizeOfTableFile);
		aux[(tableOffset - INDEXHEADER)/(sizeOfTableFile/sizeof(FAVITEM)] = aux[(sizeOfTableFile/sizeof(FAVITEM))-1];
		realloc(aux, sizeOfTableFile - sizeof(FAVITEM));
		qsort(aux, (sizeOfTableFile/sizeof(FAVITEM)), sizeof(FAVITEM), compareLanguageItem);
		if(fwrite(aux, sizeOfTableFile), 1, languageTable)) goto RTFFI_EXIT;

		rewind(languageTable);
		tableStatus = UPDATED;
		if(fwrite(&tableStatus, INDEXHEADER, 1, languageTable) <= 0) goto RTFLI_EXIT;
		return 1;
	}

	do {
		previous = current;
		if(fread(current, sizeof(FAVLISTITEM), 1, languageTable) <= 0)
			goto RTFLI_EXIT;
		if(fseek(languageList, current.fileOffset, SEEK_SET != 0) goto RTFLI_EXIT;
	} while(current.fileOffset != offset || current.next != -1);

	previous.next = current.next;
	rewind(languageTable);
	tableStatus = UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, languageTable) <= 0) goto RTFLI_EXIT;
	return 1;

RTFLI_EXIT:
	fclose(languageTable);
	fclose(languageList);
	return 0;
}

int removeTweet(char *filename, long offset) {
	if(filename == NULL) return 0;

	char *dataFileName = getDataFileName(filename);
	// tries to open the file and check if it was successful
	FILE *file = fopen(dataFileName, "r+");
	if(file == NULL) return 0;

	//saving the tweet
	TWEET *removedTweet = readTweet(filename, offset);
	//getting the stack's head
	long stackHead;
	if(fread(&stackHead, HEADER, 1, file) <= 0) goto REMOVE_TWEET_EXIT;
	
	//tries to move til the deletion's offset
	if(fseek(file, offset, SEEK_SET) != 0) goto REMOVE_TWEET_EXIT;
	int fieldSize;

	//tries to save the size of the register and moves sizeof(int) forward
	if(fread(&fieldSize, sizeof(int), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//checks if the register is already removed
	if(fieldSize <= 0) goto REMOVE_TWEET_EXIT;

	//tries to write a long that indicates the position of the last reg removed (stack's head)
	if(fwrite(&stackHead, sizeof(long), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//tries to update the stack's head in the file's beginning
	if(fseek(file, SEEK_SET, SEEK_SET) != 0) goto REMOVE_TWEET_EXIT;
	if(fwrite(&offset, sizeof(long), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//tries to back to the register and mark it as removed (negative)
	if(fseek(file, offset, SEEK_SET) != 0) goto REMOVE_TWEET_EXIT;
	fieldSize *= -1;
	if(fwrite(&fieldSize, sizeof(int), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//re-generate indexes after deleting tweet from data file
	//if(!generateIndexes(filename)) goto REMOVE_TWEET_EXIT;

	if(!removeTweetFromLanguageIndex(filename, removedTweet, offset) ||
	   !removeTweetFromFavoriteIndex(filename, removedTweet, offset))
		goto REMOVE_TWEET_EXIT;

	fclose(file);
	return 1;

REMOVE_TWEET_EXIT:
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
