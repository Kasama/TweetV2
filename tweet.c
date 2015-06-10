#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(long))	// Size of the data before the first tweet
#define minTweetSize  (4 + 2*sizeof(int) + sizeof(long))
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

typedef struct favIndexItem{

	int favoriteCount;
	long byteOffset;

}FAVITEM;

typedef struct favIndexListItem{
	
	long fileOffset;
	long next;

}FAVLISTITEM;

typedef struct langIndexItem{

	char language[MAX_LANGUAGE_SIZE];
	long byteOffset;

}LANGITEM;

typedef struct langIndexListItem{
	
	long fileOffset;
	long next;

}LANGLISTITEM;

static char *getDataFileName(char *filename){
	char *file;
	file = malloc(strlen(filename)+4);
	strcpy(file, filename);
	strcat(file, ".dat");
	return file;
}

int compareLanguageItem(const void* o1, const void* o2){
	LANGITEM *i1 = (LANGITEM*) o1;
	LANGITEM *i2 = (LANGITEM*) o2;
	return strcmp(i1->language, i2->language);
}

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
	if(arq != NULL) fclose(arq);
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
	if(arq != NULL) fclose(arq);
}

void writeTweet(char *filename, TWEET *tweet){
	if(filename == NULL || tweet == NULL) return;

	long tweetsize = tweetSize(tweet);
	long previusOffset, previusOffsetValue;
	long byteOffset  = bestFit(filename, tweetsize, &previusOffset);
	
	char * datafilename = getDataFileName(filename);
	FILE *arq = fopen(datafilename, "w+");
	if(arq == NULL) return;

	int fieldSize;
	if(fread(&fieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT;
	fieldSize *= -1;
	
	long newFieldSize = fieldSize - (tweetsize + sizeof(int));
	if(byteOffset == 0){
		flushTweet(datafilename, byteOffset, tweetsize, tweet);
	}
	else if(newFieldSize < minTweetSize){
		if(fseek(arq, byteOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT;
		if(fread(&previusOffsetValue, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT;
		if(fseek(arq, previusOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT;
		if(fwrite(&previusOffsetValue, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT;
		flushTweet(datafilename, byteOffset, fieldSize, tweet);
	}else{
		newFieldSize *= -1;			
		if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT;
		if(fwrite(&newFieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT;
		byteOffset += sizeof(int) + newFieldSize;
		flushTweet(datafilename, byteOffset, tweetsize, tweet);
	}

WRITETWEET_EXIT:
	if(arq != NULL) fclose(arq);
	if(datafilename != NULL) free(datafilename);
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

static char *readField(FILE *stream) {
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
	
	char * datafilename = getDataFileName(filename);
	dataFile = fopen(datafilename, "r");

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
	
	if(datafilename != NULL) free(datafilename);
	return tw; 
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
	long *listOffset = NULL;
	
	char * datafilename = getDataFileName(filename);
	FILE *f = fopen(datafilename, "r");
	if (f == NULL) goto OFFSERBYUSER_EXIT;
	
	TWEET *tt = NULL;
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

OFFSERBYUSER_EXIT:
	if (f != NULL) fclose(f);
	if(datafilename != NULL) free(datafilename);
	return listOffset;
}

void generateIndexes(char *filename){

	// Open all files (goto the end of this function if any errors occur)
	char *dataFileName = getDataFileName(filename);
	FILE *dataFile = fopen(dataFileName, "r");
	free(dataFileName);
	if (dataFile == NULL) return;
	char *favIdxTabFileName = getFavoriteTableIndexFileName(filename);
	FILE *favIdxTabFile = fopen(dataFileName, "w");
	free(favIdxTabFileName);
	if (favIdxTabFile == NULL) goto generateIdxData;
	char *favIdxListFileName = getFavoriteListIndexFileName(filename);
	FILE *favIdxListFile = fopen(dataFileName, "w");
	free(favIdxListFileName);
	if (favIdxListFile == NULL) goto generateIdxFTAB;
	char *langIdxTabFileName = getLanguageTableIndexFileName(filename);
	FILE *langIdxTabFile = fopen(dataFileName, "w");
	free(langIdxTabFileName);
	if (favIdxTabFile == NULL) goto generateIdxFLIST;
	char *langIdxListFileName = getLanguageListIndexFileName(filename);
	FILE *langIdxListFile = fopen(dataFileName, "w");
	free(langIdxListFileName);
	if (langIdxListFile == NULL) goto generateIdxLTAB;

	long offset;
	TWEET *current;
	fseek(dataFile, HEADER, SEEK_SET);
	fread(&offset, sizeof offset, 1, dataFile);
	if (offset > 0)
		current = readTweet(filename, offset);
	//TODO everything



generateIdxLLIST:
	fclose(langIdxListFile);
generateIdxLTAB:
	fclose(langIdxTabFile);
generateIdxFLIST:
	fclose(favIdxListFile);
generateIdxFTAB:
	fclose(favIdxTabFile);
generateIdxData:
	fclose(dataFile);

}

long *findDataOffsetByFavoriteCount(char *filename, int favoriteCount, long *foundOccurences){
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
	if (fread(&nTweets, sizeof nTweets, 1, fileTab) == 0) // Indice desatualizado
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

long *findDataOffsetByLanguage(char *filename, char* language, long *foundOccurences){
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

void destoryTweet (TWEET **tweet){
	if(tweet == NULL || *tweet == NULL)
		return;

	free((*tweet)->text);
	free((*tweet)->userName);
	free((*tweet)->coords);
	free((*tweet)->language);
	
	free(*tweet);
	tweet == NULL;
}
