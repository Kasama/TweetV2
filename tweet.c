#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(long))	// Size of the data before the first tweet
#define INDEXHEADER (sizeof(int)) // size of the date before the keys
#define UPDATED 1
#define MAX_LANGUAGE_SIZE 50

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

static char *getDataFileName(char *filename){
	char *file;
	file = malloc(strlen(filename)+4);
	strcpy(file, filename);
	strcat(file, ".dat");
	return file;
}

static char *getLanguageTableIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".itl");
	return file;
}

static char *getFavoriteTableIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".itf");
	return file;
}

static char *getLanguageListIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".ill");
	return file;
}

static char *getFavoriteListIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".ilf");
	return file;
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

typedef struct favIndexItem{

	int favoriteCount;
	long byteOffset;

}FAVITEM;

typedef struct favIndexListItem{
	
	long fileOffset;
	long next;

}FAVLISTITEM;

int compareFavoriteItem(const void* o1, const void* o2){
	FAVITEM *i1 = (FAVITEM*) o1;
	FAVITEM *i2 = (FAVITEM*) o2;
	if (i1->favoriteCount < i2->favoriteCount)
		return -1;
	else if (i1->favoriteCount == i2->favoriteCount)
		return 0;
	else
		return 1;
}

long *findOffsetByFavoriteCount(char *filename, int favoriteCount, long *foundOccurences){
	*foundOccurences = 0;
	long *ret = NULL;
	char *favIdxTabFileName = getFavoriteTableIndexFileName(filename);
	char *favIdxListFileName = getFavoriteListIndexFileName(filename);
	FILE *fileList = fopen(favIdxListFileName, "r");
	free(favIdxListFileName);
	if (fileList == NULL)
		goto findFavRet;
	FILE *fileTab = fopen(favIdxTabFileName, "r");
	free(favIdxTabFileName);
	if (fileTab == NULL)
		goto findFavRetList;

	int nTweets;
	if (fread(&nTweets, sizeof nTweets, 1, fileTab) == 0)
		goto findFavRetTab;
	if (nTweets != UPDATED)
		goto findFavRetTab;
	fseek(fileTab, 0, SEEK_END);
	nTweets = (ftell(fileTab)-INDEXHEADER)/sizeof(FAVITEM);
	FAVITEM *items = malloc(nTweets*sizeof(FAVITEM));
	if (items == NULL)
		goto findFavRetTab;
	if (fread(items, sizeof(FAVITEM), nTweets, fileTab) == 0)
		goto findFavRetTab;

	FAVITEM key;
	key.favoriteCount = favoriteCount;

	FAVITEM *found = bsearch(&key, items, nTweets, sizeof(FAVITEM), compareFavoriteItem);
	if (found == NULL)
		goto findFavRetTab;
	fseek(fileList, found->byteOffset, SEEK_SET);

	FAVLISTITEM current;
	do {
		if (fread(&current, sizeof current, 1, fileList) == 0)
			goto findFavRetTab;
		*foundOccurences++;
		ret = realloc(ret, (sizeof current)*(*foundOccurences));
		if (ret == NULL)
			goto findFavRetTab;
		ret[(*foundOccurences)-1] = current.fileOffset;
	}while(current.next != -1);

findFavRetTab:
	fclose(fileTab);
findFavRetList:
	fclose(fileList);
findFavRet:
	return ret;
}

typedef struct langIndexItem{

	char language[MAX_LANGUAGE_SIZE];
	long byteOffset;

}LANGITEM;

typedef struct langIndexListItem{
	
	long fileOffset;
	long next;

}LANGLISTITEM;

int compareLanguageItem(const void* o1, const void* o2){
	LANGITEM *i1 = (LANGITEM*) o1;
	LANGITEM *i2 = (LANGITEM*) o2;
	return strcmp(i1->language, i2->language);
}

// NEED TO BE FINISHED!!
long *findOffsetByLanguage(char *filename, char* language, long *foundOccurences){
	*foundOccurences = 0;
	long *ret = NULL;
	char *langIdxTabFileName = getLanguageTableIndexFileName(filename);
	char *langIdxListFileName = getLanguageListIndexFileName(filename);
	FILE *fileTab = fopen(langIdxTabFileName, "r");
	free(langIdxTabFileName);
	if (fileTab == NULL)
		goto findLangRet;
	FILE *fileList = fopen(langIdxListFileName, "r");
	free(langIdxListFileName);
	if (fileList == NULL)
		goto findLangRetTab;

	int nTweets;
	if (fread(&nTweets, sizeof nTweets, 1, fileTab) == 0)
		goto findLangRetList;
	if (nTweets != UPDATED)
		goto findLangRetList;
	fseek(fileTab, 0, SEEK_END);
	nTweets = (ftell(fileTab)-INDEXHEADER)/sizeof(LANGITEM);
	LANGITEM *items = malloc(nTweets*sizeof(LANGITEM));
	if (items == NULL)
		goto findLangRetList;
	if (fread(items, sizeof(LANGITEM), nTweets, fileTab) == 0)
		goto findLangRetList;

	LANGITEM key;
	int len = strlen(language);
	strncpy(key.language, language, (MAX_LANGUAGE_SIZE < len)?MAX_LANGUAGE_SIZE:len);
	key.language[MAX_LANGUAGE_SIZE-1] = 0;

	LANGITEM *found = bsearch(&key, items, nTweets, sizeof(LANGITEM), compareLanguageItem);
	if (found == NULL)
		goto findLangRetList;
	fseek(fileList, found->byteOffset, SEEK_SET);

	FAVLISTITEM current;
	char *dataFileName = getDataFileName(filename);
	FILE *dataFile = fopen(dataFileName, "r");
	free(dataFileName);
	if (dataFile == NULL)
		goto findLangRetList;
	do {
		if (fread(&current, sizeof current, 1, fileList) == 0)
			goto findLangRetData;
		fseek(dataFile, current.fileOffset, SEEK_SET);
		TWEET *t;
		if (fread(t, sizeof t, 1, dataFile) == 0)
			goto findLangRet;
		if (strcmp(t->language, language) == 0){
			ret = realloc(ret, sizeof current);
			if (ret == NULL)
				goto findLangRetData;
			ret[(*foundOccurences)++] = current.fileOffset;
		}
	}while(current.next != -1);

findLangRetData:
	fclose(dataFile);
findLangRetList:
	fclose(fileList);
findLangRetTab:
	fclose(fileTab);
findLangRet:
	return ret;
}

int removeTweet(char *filename, long offset){
	return 0;
}

void printTweet(TWEET *tweet){
	return;
}
