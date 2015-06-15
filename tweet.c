/*
 * @Authors:
 *
 * Carlos Alberto Schneider Junior - 9167910
 * Frederico de Azevedo Marques    - 8936926
 * Henrique Martins Loschiavo      - 8936972
 * Lucas Kassouf Crocomo           - 8937420
 * Roberto Pommella Alegro         - 8936756
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(long))	// Size of the data before the first tweet
#define minTweetSize  (sizeof(int) + 4 + 2*sizeof(int) + sizeof(long))
#define INDEXHEADER (sizeof(int)) // size of the date before the keys
#define UPDATED 1
#define MAX_LANGUAGE_SIZE 50

typedef struct langIndexItem{

	char language[MAX_LANGUAGE_SIZE];
	long byteOffset;

}LANGITEM;

// a generic binary search
int binarySearch(										\
	register const void *key,							\
	const void *base0,									\
	size_t nmemb,										\
	register size_t size,								\
	register int (*compar)(const void *, const void *)	\
) {

	register const char *base = base0;
	register size_t lim;
	register int cmp;
	register const void *p;

	// bsearch routine, split the number of members in half each interation
	for (lim = nmemb; lim != 0; lim >>= 1) {
		int index = (lim >> 1) * size;
		p = base + index;
		cmp = (*compar)(key, p);
		if (cmp == 0)
			return index/size;
		if (cmp > 0) {	/* key > p: move right */
			base = (char *)p + size;
			lim--;
		}		/* else move left */
	}
	return -1;
}

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

typedef struct langIndexListItem{
	
	long fileOffset;
	long next;

}LANGLISTITEM;

static char *getLanguageTableIndexFileName(char *filename);
static char *getLanguageListIndexFileName(char *filename);
static char *getFavoriteTableIndexFileName(char *filename);
static char *getFavoriteListIndexFileName(char *filename);

// Gets the name of the data (*.dat) file from the base name
static char *getDataFileName(char *filename){
	char *file;
	file = malloc(strlen(filename)+4);
	strcpy(file, filename);
	strcat(file, ".dat");
	return file;
}

// compare 2 table items and check if they are less than, equal to or greater than the other
int compareLanguageItem(const void* o1, const void* o2){
	LANGITEM *i1 = (LANGITEM*) o1;
	LANGITEM *i2 = (LANGITEM*) o2;
	return strcmp(i1->language, i2->language);
}

// compare 2 table items and check if they are less than, equal to or greater than the other
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

static long bestFit(char* filename, long tweetSize, long *minPreviousOffset){

	long offset, stackPos, currentPreviousOffset;
	int currentDifference, minDifference, blockSize;
	*minPreviousOffset = 0;

	FILE *arq = fopen(filename, "r");
	if(arq == NULL)
		return -1;

	if(fread(&stackPos, sizeof(long), 1, arq) <= 0) goto BESTFIT_ERROR;

	if(stackPos <= 0){
		if(arq !=NULL)
			fclose(arq);
		return -1;
	}

	minDifference = INT_MAX;
	currentPreviousOffset = -1;
	offset = -1;
	while(stackPos != -1 && minDifference > 0){
		if(fseek(arq, stackPos, SEEK_SET) != 0) goto BESTFIT_ERROR;

		if(fread(&blockSize, sizeof(int), 1, arq) <= 0) goto BESTFIT_ERROR;
		blockSize = abs(blockSize);
		currentDifference = blockSize - tweetSize;

		if(currentDifference >= 0 && currentDifference < minDifference){
			minDifference = currentDifference;
			*minPreviousOffset = currentPreviousOffset;
			offset = stackPos;
		}
		currentPreviousOffset = stackPos;	
		if(fread(&stackPos, sizeof(long), 1, arq) <= 0) goto BESTFIT_ERROR;
	}

BESTFIT_ERROR:
	if(arq != NULL) fclose(arq);
	return offset;
}

// get the size in bytes of a tweet in memmory
static int tweetSize(TWEET* tweet){
	int sum = 0;
	sum += strlen(tweet->text);
	sum += strlen(tweet->userName);
	sum += strlen(tweet->coords);
	sum += strlen(tweet->language);
	sum += 2*sizeof(int) + sizeof(long);
	
	return sum;
}

//write a tweet to an file
static void flushTweet(char* filename, long byteOffset, int fieldsize, TWEET *tweet){

	FILE *arq = NULL;
	//if the file is not created, it will be created and then reopen to allow moving around the file's cursor
	arq = fopen(filename, "a");
	arq = freopen(filename, "r+", arq );
	if(arq == NULL) return;

	//move to the given location in the file and starts to write the tweet
	if(byteOffset != 0){
		if(fseek(arq, byteOffset, SEEK_SET) != 0)	goto FLUSHTWEET_EXIT;
	}
	if(fwrite(&fieldsize, sizeof fieldsize, 1, arq) <= 0)	goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->text, sizeof(char), strlen(tweet->text), arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->userName, sizeof(char), strlen(tweet->userName), arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->coords, sizeof(char), strlen(tweet->coords), arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(tweet->language, sizeof(char), strlen(tweet->language), arq) <= 0)		goto FLUSHTWEET_EXIT;
	if(fwrite(&(tweet->favoriteCount), sizeof(int), 1, arq) <= 0)	goto FLUSHTWEET_EXIT;
	if(fwrite(&(tweet->retweetCount), sizeof(int), 1, arq) <= 0)	goto FLUSHTWEET_EXIT;
	if(fwrite(&(tweet->viewsCount), sizeof(long), 1, arq) <= 0)	goto FLUSHTWEET_EXIT;

FLUSHTWEET_EXIT:
	if(arq != NULL) fclose(arq);
}

//this function will update both, list and table, files from Language Index
static void updateLanguageIndexFiles(char* table, char* list, TWEET *tweet, long byteOffset){
	
	//create if needed a new file and then reopen to work on it
	FILE *indexTable = fopen(table, "a");
	indexTable = freopen(table, "r+", indexTable);
	FILE *indexList = NULL;
	int updatedStatus = !UPDATED;//set updated file status variable to "not updated"

	fseek(indexTable, 0, SEEK_END);//go to end of file
	long fileSize = ftell(indexTable);//get the file's size
	fseek(indexTable, INDEXHEADER, SEEK_SET);//move to the frist entry on file, which is after the header
	LANGITEM *langVector = malloc(fileSize - INDEXHEADER);//allocate a vector for table
	LANGITEM newEntryOnTab;	//new entry on tab variable
	LANGLISTITEM newEntryOnList;//new entry on list variable
	int size = (fileSize - INDEXHEADER)/sizeof(LANGITEM);//get the vector's size 
	if (size < 0) size = 0;
	
	//create the key. if it's larger than 50 units, than it will be cropped.
	int len = strlen(tweet->language);
	len = (MAX_LANGUAGE_SIZE < len)?MAX_LANGUAGE_SIZE:len;
	strncpy(newEntryOnTab.language, tweet->language, len);
	newEntryOnTab.language[len - 1] = 0;	

	//load table file to memory, using a vector
	fread(langVector, sizeof(LANGITEM), size, indexTable);
	int index = binarySearch(&newEntryOnTab, langVector, size, sizeof(LANGITEM), compareLanguageItem);//returns the key's index
	int found = index;

	if(found == -1){//if we dont have the given key
		langVector = realloc(langVector, (fileSize - INDEXHEADER + sizeof(LANGITEM))); //add a new space to add the new entry
		size++;
		langVector[size - 1].byteOffset = -1; //append insert variable at the vector's end
		strcpy(langVector[size - 1].language, newEntryOnTab.language);
		index = size - 1;
	}

	indexList = fopen(list, "a"); //open list file to append
	indexList = freopen(list, "r+", indexList);
	fseek(indexList, 0, SEEK_SET);
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);//set file to not updated
	fseek(indexList, 0, SEEK_END);//go to eof
	
	//stores the beytoffset in the entry's table variable
	newEntryOnTab.byteOffset = ftell(indexList);
	newEntryOnList.fileOffset = byteOffset;
	newEntryOnList.next = langVector[index].byteOffset;
	
	//write in the list's file the new entry
	fwrite(&newEntryOnList, sizeof newEntryOnList, 1, indexList);
	langVector[index].byteOffset = newEntryOnTab.byteOffset;

	//if we didn't found the key, QuickSort the vector
	if(found == -1)
		qsort(langVector,size, sizeof(LANGITEM), compareLanguageItem);//ordeno o vetor
	
	//setting updated status to uptaded
	updatedStatus = UPDATED;	
	fclose(indexTable);

	//writing to file
	indexTable = fopen(table, "w+");
	fwrite(&updatedStatus, sizeof(int), 1, indexTable);	
	fwrite(langVector, sizeof(LANGITEM), size, indexTable);

	//setting updated status
	fseek(indexList, 0, SEEK_SET);
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);

	//closing all files and retur memory to OS
	fclose(indexTable);
	fclose(indexList);
	free(langVector);
}

//this function will update both, list and table, files from favorite count Index
static void updateFavoriteCountIndexFiles(char* table, char* list, TWEET *tweet, long byteOffset){

	//create if needed a new file and then reopen to work on it
	FILE *indexTable = fopen(table, "a");
	indexTable = freopen(table,"r+",indexTable );
	FILE *indexList = NULL;
	int updatedStatus = !UPDATED;//set updated file status variable to "not updated"

	fseek(indexTable, 0, SEEK_END);//go to end of file
	long fileSize = ftell(indexTable);//get the file's size
	fseek(indexTable, INDEXHEADER, SEEK_SET);//move to the frist entry on file, which is after the header
	FAVITEM *favVector = malloc(fileSize - INDEXHEADER);//allocate a vector for table
	FAVITEM newEntryOnTab;//new entry on tab variable
	FAVLISTITEM newEntryOnList;//new entry on list variable
	int size = (fileSize - INDEXHEADER)/sizeof(FAVITEM);//get the vector's size 
	if (size < 0) size = 0;
	
	newEntryOnTab.favoriteCount = tweet->favoriteCount;
	
	//load table file to memory, using a vector
	fread(favVector, sizeof(FAVITEM), size, indexTable);

	int index = binarySearch(&newEntryOnTab, favVector, size, sizeof(FAVITEM), compareFavoriteItem);	
	int found = index;

	if(found == -1){//if we dont have the given key
		favVector = realloc(favVector, (fileSize - sizeof(int) + sizeof(FAVITEM))); //add a new space to add the new entry
		size++;
		favVector[size - 1].byteOffset = -1; //append insert variable at the vector's end
		favVector[size - 1].favoriteCount = newEntryOnTab.favoriteCount;
		index = size - 1;
	}

	indexList = fopen(list, "a"); //open list file to append
	indexList = freopen(list,"r+",indexList );
	fseek(indexList, 0, SEEK_SET);
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);//set file to not updated
	fseek(indexList, 0, SEEK_END);//go to eof

	//stores the beytoffset in the entry's table variable
	newEntryOnTab.byteOffset = ftell(indexList);
	newEntryOnList.fileOffset = byteOffset;
	newEntryOnList.next = favVector[index].byteOffset;

	//write in the list's file the new entry
	fwrite(&newEntryOnList, sizeof newEntryOnList, 1, indexList);
	favVector[index].byteOffset = newEntryOnTab.byteOffset;

	//if we didn't found the key, QuickSort the vector
	if(found == -1)
		qsort(favVector,size, sizeof(FAVITEM), compareFavoriteItem);//ordeno o vetor
	
	//setting updated status to uptaded
	updatedStatus = UPDATED;	
	fclose(indexTable);

	//writing to file
	indexTable = fopen(table, "w+");
	fwrite(&updatedStatus, sizeof(int), 1, indexTable);	
	fwrite(favVector, sizeof(FAVITEM), size, indexTable);

	//setting updated status
	fseek(indexList, 0, SEEK_SET);
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);

	fclose(indexTable);
	fclose(indexList);
	free(favVector);
}

static void updateIndexFiles(TWEET *tweet, long byteOffset, char* filename){
	
	char *table = getLanguageTableIndexFileName(filename);//generating table file path
	char *list = getLanguageListIndexFileName(filename);//generating list file path
	
	//updates Language index file
	updateLanguageIndexFiles (table, list, tweet, byteOffset);

	free(list);
	free(table);

	//regenerating path's
	table = getFavoriteTableIndexFileName(filename);
	list = getFavoriteListIndexFileName(filename);

	//updating files
	updateFavoriteCountIndexFiles(table, list, tweet, byteOffset);

	free(list);
	free(table);
}

int writeTweet(char *filename, TWEET *tw){
	if(filename == NULL || tw == NULL) return 0;

	//creating new tweet variable
	TWEET *tweet;
	tweet = malloc(sizeof(TWEET));

	//creating space for file separators
	tweet->text = malloc((strlen(tw->text)) + 2);
	tweet->userName = malloc((strlen(tw->userName)) + 2);
	tweet->coords = malloc((strlen(tw->coords)) + 2);
	tweet->language = malloc((strlen(tw->language)) + 2);

	//copying information from input to variable
	strcpy(tweet->text, tw->text);
	strcpy(tweet->userName, tw->userName);
	strcpy(tweet->coords, tw->coords);
	strcpy(tweet->language, tw->language);

	//append separatos and strings endings
	tweet->text[strlen(tw->text)] = END_FIELD;
	tweet->text[strlen(tw->text)+1] = '\0';
	tweet->userName[strlen(tw->userName)] = END_FIELD;
	tweet->userName[strlen(tw->userName)+1] = '\0';
	tweet->coords[strlen(tw->coords)] = END_FIELD;
	tweet->coords[strlen(tw->coords)+1] = '\0';
	tweet->language[strlen(tw->language)] = END_FIELD;
	tweet->language[strlen(tw->language)+1] = '\0';
	
	//finish copy information
	tweet->favoriteCount = tw->favoriteCount;
	tweet->retweetCount = tw->retweetCount;
	tweet->viewsCount = tw->viewsCount;

	
	int tweetsize = tweetSize(tweet);  //insert tweets size
	long previousOffset, byteOffset;
	long stackHead = 0;
	
	char *datafilename = getDataFileName(filename); //data file name
	FILE *arq = fopen(datafilename, "a");
	arq = freopen(datafilename, "r+", arq);

	if(arq == NULL) return 0;
	fseek(arq, 0, SEEK_END);
	if(ftell(arq) == 0){
		stackHead = -1;
		fwrite(&stackHead, sizeof stackHead, 1, arq);
	}
	fseek(arq, 0, SEEK_SET);

	if(stackHead == 0)
		if(fread(&stackHead, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT; //romved stack's head
	
	if(stackHead == -1){ // there is no logical removed space
		if(fseek(arq, 0, SEEK_END) != 0) goto WRITETWEET_EXIT;
		byteOffset = ftell(arq);
		//append tweet
		flushTweet(datafilename, byteOffset, tweetsize, tweet);
	}else{//logical removed space exists
		byteOffset = bestFit(datafilename, tweetsize, &previousOffset); //insert's tweet byteoffset
		int fieldSize;
		if(byteOffset == -1){ //there is no free space for tweet, append it
			if(fseek(arq, 0, SEEK_END) != 0) goto WRITETWEET_EXIT;
			byteOffset = ftell(arq);
		}else{ //there is a space to insert the tweet
			if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT; //moving to insert's position
			if(fread(&fieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT; //reading inserting field size
			//comparing fieldsize with tweet size
			fieldSize = abs(fieldSize);
			int newFieldSize = fieldSize - (tweetsize + sizeof(int));
			if(previousOffset == -1){
				previousOffset = 0;
			}else{
				previousOffset = previousOffset - sizeof(int);
			}

			if(newFieldSize < minTweetSize){ //inserting fieldsize is not enough, so ew add it to tweet insertion
				long nextOffset;
				if(fseek(arq, byteOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT; //moving to long to read field size 
				if(fread(&nextOffset, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT; //reading field size
				if(fseek(arq, previousOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT; //moving cursor to previus field
				if(fwrite(&nextOffset, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT; //writing next position on previous field
				if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT;
			}else{//field is too small
				if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT;//moving cursor to field's start position
				byteOffset += sizeof(int) + newFieldSize;//updating byteoffset to write the tweet at the field's end
				newFieldSize = -abs(newFieldSize);//updating fieldsize to logical removed (negative size)			
				if(fwrite(&newFieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT;//writing new size
			}
		}
		flushTweet(datafilename, byteOffset, tweetsize, tweet);//writing tweet to file

	}
	//updating indeces
	updateIndexFiles(tweet, byteOffset, filename);

	if(arq != NULL) fclose(arq);
	if(datafilename != NULL) free(datafilename);
	if(tweet != NULL) free(tweet);
	return 1;

WRITETWEET_EXIT:
	if(arq != NULL) fclose(arq);
	if(datafilename != NULL) free(datafilename);
	if(tweet != NULL) free(tweet);
	return 0;

}

// allocates a new tweet from the given data
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

	tw->text = malloc((strlen(text)+1));
	tw->userName = malloc((strlen(userName)+1));
	tw->coords = malloc((strlen(coords)+1));
	tw->language = malloc((strlen(language)+1));

	strcpy(tw->text, text);
	strcpy(tw->userName, userName);
	strcpy(tw->coords, coords);
	strcpy(tw->language, language);

	tw->favoriteCount = favoriteCount;
	tw->retweetCount = retweetCount;
	tw->viewsCount = viewsCount;

	return tw;
}

// reads a field until the delimiter "END_FIELD" is found
static char *readField(FILE *stream) {
	char *buffer = NULL;
	char character;
	int counter = 0;

	do {
		character = fgetc(stream);
		buffer = (char *) realloc(buffer, sizeof(char)
				* (counter+1));
		buffer[counter++] = character;
	}while (character != END_FIELD);
	buffer[counter-1] = '\0';

	return buffer;
}

TWEET *readTweet(char *filename, long offset){
	FILE *dataFile;
	TWEET *tw = malloc(sizeof(TWEET));
	if(tw == NULL) return NULL;

	//gets the data file name and opens it
	char *datafilename = getDataFileName(filename);
	if(datafilename == NULL) return NULL;

	dataFile = fopen(datafilename, "r");
	if(dataFile == NULL) return NULL;

	if(fseek(dataFile, offset, SEEK_SET) != 0) return NULL;

	int size;
	if(fread(&size, sizeof(int), 1, dataFile) <= 0) return NULL;

	if (size < 0) return NULL;
	
	void *dataRaw;
	dataRaw = malloc(size);
	if(dataRaw == NULL) return NULL;

	//loading the tweet to memory
	if(fread(dataRaw, size, 1, dataFile) <= 0) return NULL;

	//creates a memory buffer
	FILE *mem;
	mem = fmemopen(dataRaw, size, "r");
	if(mem == NULL) return NULL;

	//loading the tweet fields
	tw->text = readField(mem);
	tw->userName = readField(mem);
	tw->coords = readField(mem);
	tw->language = readField(mem);
	if(fread(&(tw->favoriteCount), sizeof(int), 1, mem) <= 0) return NULL; 
	if(fread(&(tw->retweetCount), sizeof(int), 1, mem) <= 0) return NULL; 
	if(fread(&(tw->viewsCount), sizeof(long), 1, mem) <= 0) return NULL;

	free(datafilename);
	
	//returning it
	return tw; 
}

//get a table index filename for language using the base filename
static char *getLanguageTableIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".itl");
	return file;
}

//get a table index filename for favorite using the base filename
static char *getFavoriteTableIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".itf");
	return file;
}

//get a list index filename for language using the base filename
static char *getLanguageListIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".ill");
	return file;
}

//get a list index filename for favorite using the base filename
static char *getFavoriteListIndexFileName(char *filename){
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".ilf");
	return file;
}

// goes through the data file and return all tweets
long *findAllTweets(char *filename, long *foundOccurences){
	*foundOccurences = 0;
	long *listOffset = NULL;

	char * datafilename = getDataFileName(filename);
	FILE *f = fopen(datafilename, "r");
	//open the file

	if (f == NULL) goto FINDALLTWEETS_EXIT;

	TWEET *tt = NULL;
	long offset = HEADER;		// Offset of the last begin of tweet
	int nextTweet = 0;			// Offset from the last begin of tweet until the next begin of tweet

	fseek(f, offset, SEEK_SET);
	// for every tweet in the file
	while(fread(&nextTweet, 
				sizeof nextTweet, 1, f) > 0 ) {
		// that is not removed
		if (nextTweet > 0){
			// add it to the list of found tweets
			(*foundOccurences)++;
			listOffset = realloc(listOffset, *foundOccurences);
			listOffset[(*foundOccurences)-1] = offset;
		}

		offset += abs(nextTweet) + sizeof nextTweet;
		fseek(f, offset, SEEK_SET);
	}

FINDALLTWEETS_EXIT:
	if (f != NULL) fclose(f);
	if(datafilename != NULL) free(datafilename);
	//return the list of found
	return listOffset;
}

long *findOffsetByUser(char *filename, char *username, long *foundOccurences){
	*foundOccurences = 0;
	long *listOffset = NULL;

	char *datafilename = getDataFileName(filename);
	if(datafilename == NULL) return NULL;
	
	FILE *f = fopen(datafilename, "r");
	if (f == NULL) goto OFFSETBYUSER_EXIT;

	TWEET *tt = NULL;
	long offset = HEADER;		// Offset of the last begin of tweet
	int nextTweet = 0;			// Offset from the last begin of tweet until the next begin of tweet

	//goes to the beginning of the tweets in the data file
	fseek(f, offset, SEEK_SET);
	//while tweets exist
	while(fread(&nextTweet, 
				sizeof nextTweet, 1, f) > 0 ) {
		if (nextTweet > 0){
			//loads the tweet
			tt = readTweet(filename, offset);
			if(tt == NULL) return NULL;
			//if there is a match by user
			int cmp = strcmp(username, tt->userName);
			if (cmp == 0) {
				//increase the size of listOffset vector and adds the offset to its end
				(*foundOccurences)++;
				listOffset = realloc(listOffset, *foundOccurences);
				listOffset[(*foundOccurences)-1] = offset;
			}
			//frees the tweet
			destroyTweet(&tt);
		}
		//updates offset to the next tweet
		offset += abs(nextTweet) + sizeof nextTweet;
		//goes to the next tweet
		fseek(f, offset, SEEK_SET);
	}

OFFSETBYUSER_EXIT:
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
	FILE *favIdxTabFile = fopen(dataFileName, "w+");
	if (favIdxTabFile != NULL) fclose(favIdxTabFile);
	free(favIdxTabFileName);
	char *favIdxListFileName = getFavoriteListIndexFileName(filename);
	FILE *favIdxListFile = fopen(dataFileName, "w+");
	if (favIdxListFile != NULL) fclose(favIdxListFile);
	free(favIdxListFileName);
	char *langIdxTabFileName = getLanguageTableIndexFileName(filename);
	FILE *langIdxTabFile = fopen(dataFileName, "w+");
	if (langIdxTabFile != NULL) fclose(langIdxTabFile);
	free(langIdxTabFileName);
	char *langIdxListFileName = getLanguageListIndexFileName(filename);
	FILE *langIdxListFile = fopen(dataFileName, "w+");
	if (langIdxListFile != NULL) fclose(langIdxListFile);
	free(langIdxListFileName);

	long offset;
	TWEET *current;
	fseek(dataFile, HEADER, SEEK_SET);
	while(fread(&offset, sizeof offset, 1, dataFile) > 0){
		fseek(dataFile, abs(offset), SEEK_CUR);
		if (offset > 0){
			current = readTweet(filename, offset);
			updateIndexFiles(current, offset, filename);
		}
	}

}

static long findIndexOffsetByFavoriteCount(char *filename, int favoriteCount){
	int i;
	long ret = -1;
	char *favIdxTabFileName = getFavoriteTableIndexFileName(filename);
	FILE *fileTab = fopen(favIdxTabFileName, "r");
	free(favIdxTabFileName);
	if (fileTab == NULL)
		goto findFavRet;

	int nTweets;
	if (fread(&nTweets, sizeof nTweets, 1, fileTab) <= 0) // Indice desatualizado
		goto findFavRetTab;
	if (nTweets != UPDATED)
		goto findFavRetTab;
	fseek(fileTab, 0, SEEK_END);
	nTweets = (ftell(fileTab)-INDEXHEADER)/sizeof(FAVITEM);
	fseek(fileTab, INDEXHEADER, SEEK_SET);
	FAVITEM *items = malloc(nTweets*sizeof(FAVITEM));
	if (items == NULL)
		goto findFavRetTab;
	if (fread(items, sizeof(FAVITEM), nTweets, fileTab) <= 0)
		goto findFavRetTab;

	FAVITEM key;
	key.favoriteCount = favoriteCount;
	for (i = 0; i < nTweets; i++){
	}
	int found = binarySearch(&key, items, nTweets, sizeof(FAVITEM), compareFavoriteItem);
	if (found == -1)
		goto findFavRetTab;
	ret = (found*sizeof(FAVITEM))+INDEXHEADER;

findFavRetTab:
	fclose(fileTab);
findFavRet:
	return ret;
}

static long findIndexOffsetByLanguage(char *filename, char* language){
	long ret = -1;
	char *langIdxTabFileName = getLanguageTableIndexFileName(filename);
	FILE *fileTab = fopen(langIdxTabFileName, "r");
	free(langIdxTabFileName);
	if (fileTab == NULL)
		goto findLangRet;

	int nTweets;
	if (fread(&nTweets, sizeof nTweets, 1, fileTab) <= 0)
		goto findLangRetTab;
	if (nTweets != UPDATED)
		goto findLangRetTab;
	fseek(fileTab, 0, SEEK_END);
	nTweets = (ftell(fileTab)-INDEXHEADER)/sizeof(LANGITEM);
	fseek(fileTab, INDEXHEADER, SEEK_SET);
	LANGITEM *items = malloc(nTweets*sizeof(LANGITEM));
	if (items == NULL)
		goto findLangRetTab;
	if (fread(items, sizeof(LANGITEM), nTweets, fileTab) <= 0)
		goto findLangRetTab;

	LANGITEM key;
	int len = strlen(language);
	len = (MAX_LANGUAGE_SIZE < len)?MAX_LANGUAGE_SIZE:len;
	strncpy(key.language, language, len);
	key.language[len] = 0;
	int found = binarySearch(&key, items, nTweets, sizeof(LANGITEM), compareLanguageItem);
	if (found == -1)
		goto findLangRetTab;

	ret = (found*sizeof(LANGITEM))+INDEXHEADER;

findLangRetTab:
	fclose(fileTab);
findLangRet:
	return ret;
}

// Find all data offsets with certain favorite count
long *findDataOffsetByFavoriteCount(char *filename, int favoriteCount, long *foundOccurences){
	*foundOccurences = 0;
	long *ret = NULL;	// Default return

	// Get file names, open the files, and free the generated filenames
	char *listFileName = getFavoriteListIndexFileName(filename);
	char *tabFileName = getFavoriteTableIndexFileName(filename);
	FILE *fileList = fopen(listFileName, "r");
	FILE *fileTab = fopen(tabFileName, "r");

	free(listFileName);
	if (fileList == NULL) goto findFavRet;
	free(tabFileName);
	if (fileTab == NULL) goto findFavRetList;

	long offset;

	// Get first item on stack at List Index File
	offset = findIndexOffsetByFavoriteCount(filename, favoriteCount);
	if (offset == -1) goto findFavRetList;
	fseek(fileTab, offset, SEEK_SET);
	FAVITEM i;
	fread(&i, sizeof i, 1, fileTab);
	fseek(fileList, i.byteOffset, SEEK_SET);

	// Load the list in *ret
	FAVLISTITEM current;
	do {
		if (fread(&current, sizeof current, 1, fileList) <= 0) goto findFavRetTab;
		(*foundOccurences)++;
		ret = realloc(ret, (sizeof current)*(*foundOccurences));
		if (ret == NULL) goto findFavRetTab;
		ret[(*foundOccurences)-1] = current.fileOffset;
		if (current.next > 0) fseek(fileList, current.next, SEEK_SET);
	}while(current.next != -1);

// Close files
findFavRetTab:
	fclose(fileTab);
findFavRetList:
	fclose(fileList);
findFavRet:
	return ret;
}

// Find all data offsets with certain language
long *findDataOffsetByLanguage(char *filename, char* language, long *foundOccurences){
	*foundOccurences = 0;
	long *ret = NULL;	// Default return

	// Get file names, open the files, and free the generated filenames
	char *langIdxTabFileName = getLanguageTableIndexFileName(filename);
	char *langIdxListFileName = getLanguageListIndexFileName(filename);
	FILE *fileTab = fopen(langIdxTabFileName, "r");
	FILE *fileList = fopen(langIdxListFileName, "r");

	free(langIdxTabFileName);
	if (fileTab == NULL) goto findLangRet;
	free(langIdxListFileName);
	if (fileList == NULL) goto findLangRetTab;

	int nTweets;
	// Read List status and check if it's updated
	if (fread(&nTweets, sizeof nTweets, 1, fileTab) == 0) goto findLangRetList;
	if (nTweets != UPDATED) goto findLangRetList;

	// Get the number of items in Table File (without header)
	fseek(fileTab, 0, SEEK_END);
	nTweets = (ftell(fileTab)-INDEXHEADER)/sizeof(LANGITEM);

	// Loads Table File to RAM (without header)
	fseek(fileTab, INDEXHEADER, SEEK_SET);
	LANGITEM *items = malloc(nTweets*sizeof(LANGITEM));
	if (items == NULL) goto findLangRetList;
	if (fread(items, sizeof(LANGITEM), nTweets, fileTab) == 0) goto findLangRetList;

	long offset;

	// Get first item on stack at List Index File
	offset = findIndexOffsetByLanguage(filename, language);
	if (offset == -1) goto findLangRetList;
	fseek(fileTab, offset, SEEK_SET);
	LANGITEM i;
	fread(&i, sizeof i, 1, fileTab);
	fseek(fileList, i.byteOffset, SEEK_SET);

	// Load the list in *ret
	LANGLISTITEM current;
	do {
		if (fread(&current, sizeof current, 1, fileList) == 0) goto findLangRetList;
		TWEET *t = readTweet(filename, current.fileOffset);
		if (strcmp(t->language, language) == 0){	// Compare if really match the key with the data file
			ret = realloc(ret, sizeof current);
			if (ret == NULL) goto findLangRetList;
			ret[(*foundOccurences)++] = current.fileOffset;
		}
		if (current.next > 0) fseek(fileList, current.next, SEEK_SET);
	}while(current.next != -1);

// Close files
findLangRetList:
	fclose(fileList);
findLangRetTab:
	fclose(fileTab);
findLangRet:
	return ret;
}

// Remove Tweet from language index files
static int removeTweetFromFavoriteIndex(char *filename, TWEET *removedTweet, long offset) {
	// Getting the proper filename and open files
	char *tableFileName = getFavoriteTableIndexFileName(filename);
	char *listFileName  = getFavoriteListIndexFileName(filename);
	FILE *favoriteTable = fopen(tableFileName, "r+");
	FILE *favoriteList  = fopen(listFileName,  "r+");
	if(favoriteTable == NULL || favoriteList == NULL || removedTweet == NULL) return 0;

	//marking table status as not updated
	int tableStatus = !UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
	if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteList) <= 0) goto RTFFI_EXIT;

	//getting the offset of the tweet in the table index file
	long tableOffset = findIndexOffsetByFavoriteCount(filename, removedTweet->favoriteCount);

	//getting the byteOffset for the item in the list index file
	FAVITEM tableItem;
	if(fseek(favoriteTable, tableOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(&tableItem, sizeof(FAVITEM), 1, favoriteTable) <= 0) goto RTFFI_EXIT;

	//going to the list index file 
	FAVLISTITEM current, previous;
	if(fseek(favoriteList, tableItem.byteOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(&current, sizeof(FAVLISTITEM), 1, favoriteList) <= 0) goto RTFFI_EXIT;

	// IF the current item is the first or only element in stack
	if(current.next == -1 || current.fileOffset == offset){
		tableItem.byteOffset = current.next;	// Points to the next element (or "-1" if it is the only)

		if (tableItem.byteOffset == -1){	// IF it is the only element...
			FAVITEM *table;

			// Get the size of file (without header)
			if(fseek(favoriteTable, 0, SEEK_END) != 0) goto RTFFI_EXIT;
			long tableSize = ftell(favoriteTable) - INDEXHEADER;

			// Goes to first element in table File and load it to memory
			if(fseek(favoriteTable, INDEXHEADER, SEEK_SET) != 0) goto RTFFI_EXIT;
			table = malloc(tableSize);
			if(table == NULL) goto RTFFI_EXIT;
			int nItems = tableSize/sizeof(FAVITEM);
			if(fread(table, sizeof(FAVITEM), nItems, favoriteTable) <= 0) goto RTFFI_EXIT;

			// Find the item to remove (the index in *table)
			int toRemoveIndex = (tableOffset - INDEXHEADER)/sizeof(FAVITEM);
			table[toRemoveIndex] = table[nItems-1];	// table[removedItem] = table[lastItem]
			nItems--;	// Decrease size of *table

			// Re-write table file
			favoriteTable = freopen(tableFileName,"w+",favoriteTable);
			if(fwrite(&tableStatus, sizeof tableStatus, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
			if (nItems != 0){	// Sort the vector if its greater than zero
				qsort(table, nItems, sizeof(FAVITEM), compareFavoriteItem);
				if(fwrite(table, sizeof(FAVITEM), nItems, favoriteTable) <= 0) goto RTFFI_EXIT;
			}

		}else{	// IF it has more than one element, re-write the table File
			if(fseek(favoriteTable, tableOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
			if(fwrite(&tableItem, sizeof tableItem, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
		}
	}else{	// IF the current item IS NOT the first NOR only element in stack
		long toUpdate;

		do {	// Search the element to be removed in index list file
			previous = current;
			toUpdate = ftell(favoriteList) - sizeof(FAVLISTITEM);
			if(fseek(favoriteList, current.next, SEEK_SET) != 0) goto RTFFI_EXIT;
			if(fread(&current, sizeof current, 1, favoriteList) <= 0) goto RTFFI_EXIT;
		}while(current.fileOffset != offset || current.next != -1);

		// Remove logically the element in list by changing pointers
		previous.next = current.next;
		if(fseek(favoriteList, toUpdate, SEEK_SET) != 0) goto RTFFI_EXIT;
		if(fwrite(&previous, sizeof previous, 1, favoriteList) <= 0) goto RTFFI_EXIT;
	}

	// Update the status
	if(fseek(favoriteTable, 0, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fseek(favoriteList, 0, SEEK_SET) != 0) goto RTFFI_EXIT;
	tableStatus = !tableStatus;
	if(fwrite(&tableStatus, sizeof tableStatus, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
	if(fwrite(&tableStatus, sizeof tableStatus, 1, favoriteList) <= 0) goto RTFFI_EXIT;

	// Close files
	fclose(favoriteTable);
	fclose(favoriteList);
	free(listFileName);
	free(tableFileName);
	return 1;

RTFFI_EXIT:
	fclose(favoriteTable);
	fclose(favoriteList);
	free(listFileName);
	free(tableFileName);
	return 0;
}

// Remove Tweet from language index files
static int removeTweetFromLanguageIndex(char *filename, TWEET *removedTweet, long offset) {
	// Getting the proper filename and open files
	char *tableFileName = getLanguageTableIndexFileName(filename);
	char *listFileName  = getLanguageListIndexFileName(filename);
	FILE *languageTable = fopen(tableFileName, "r+");
	FILE *languageList  = fopen(listFileName,  "r+");
	if(languageTable == NULL || languageList == NULL || removedTweet == NULL) return 0;

	//marking table status as not updated
	int tableStatus = !UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, languageTable) <= 0) goto RTFFI_EXIT;
	if(fwrite(&tableStatus, INDEXHEADER, 1, languageList) <= 0) goto RTFFI_EXIT;

	//getting the offset of the tweet in the table index file
	long tableOffset = findIndexOffsetByLanguage(filename, removedTweet->language);

	//getting the byteOffset for the item in the list index file
	LANGITEM tableItem;
	if(fseek(languageTable, tableOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(&tableItem, sizeof(LANGITEM), 1, languageTable) <= 0) goto RTFFI_EXIT;

	//going to the list index file 
	LANGLISTITEM current, previous;
	if(fseek(languageList, tableItem.byteOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(&current, sizeof(LANGLISTITEM), 1, languageList) <= 0) goto RTFFI_EXIT;

	// IF the current item is the first or only element in stack
	if(current.next == -1 || current.fileOffset == offset){
		tableItem.byteOffset = current.next;	// Points to the next element (or "-1" if it is the only)

		if (tableItem.byteOffset == -1){	// IF it is the only element...
			LANGITEM *table;

			// Get the size of file (without header)
			if(fseek(languageTable, 0, SEEK_END) != 0) goto RTFFI_EXIT;
			long tableSize = ftell(languageTable) - INDEXHEADER;

			// Goes to first element in table File and load it to memory
			if(fseek(languageTable, INDEXHEADER, SEEK_SET) != 0) goto RTFFI_EXIT;
			table = malloc(tableSize);
			if(table == NULL) goto RTFFI_EXIT;
			int nItems = tableSize/sizeof(LANGITEM);
			if(fread(table, sizeof(LANGITEM), nItems, languageTable) <= 0) goto RTFFI_EXIT;

			// Find the item to remove (the index in *table)
			int toRemoveIndex = (tableOffset - INDEXHEADER)/sizeof(LANGITEM);
			table[toRemoveIndex] = table[nItems-1];	// table[removedItem] = table[lastItem]
			nItems--;	// Decrease size of *table

			// Re-write table file
			languageTable = freopen(tableFileName,"w+",languageTable);
			if(fwrite(&tableStatus, sizeof tableStatus, 1, languageTable) <= 0) goto RTFFI_EXIT;
			if (nItems != 0){	// Sort the vector if its greater than zero
				qsort(table, nItems, sizeof(LANGITEM), compareLanguageItem);
				if(fwrite(table, sizeof(LANGITEM), nItems, languageTable) <= 0) goto RTFFI_EXIT;
			}

		}else{	// IF it has more than one element, re-write the table File
			if(fseek(languageTable, tableOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
			if(fwrite(&tableItem, sizeof tableItem, 1, languageTable) <= 0) goto RTFFI_EXIT;
		}

	}else{	// IF the current item IS NOT the first NOR only element in stack
		long toUpdate;

		do {	// Search the element to be removed in index list file
			previous = current;
			toUpdate = ftell(languageList) - sizeof(LANGLISTITEM);
			if(fseek(languageList, current.next, SEEK_SET) != 0) goto RTFFI_EXIT;
			if(fread(&current, sizeof current, 1, languageList) <= 0) goto RTFFI_EXIT;
		}while(current.fileOffset != offset || current.next != -1);

		// Remove logically the element in list by changing pointers
		previous.next = current.next;
		if(fseek(languageList, toUpdate, SEEK_SET) != 0) goto RTFFI_EXIT;
		if(fwrite(&previous, sizeof previous, 1, languageList) <= 0) goto RTFFI_EXIT;
	}

	// Update the status
	if(fseek(languageTable, 0, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fseek(languageList, 0, SEEK_SET) != 0) goto RTFFI_EXIT;
	tableStatus = !tableStatus;
	if(fwrite(&tableStatus, sizeof tableStatus, 1, languageTable) <= 0) goto RTFFI_EXIT;
	if(fwrite(&tableStatus, sizeof tableStatus, 1, languageList) <= 0) goto RTFFI_EXIT;

	// Close files
	fclose(languageTable);
	fclose(languageList);
	free(listFileName);
	free(tableFileName);
	return 1;

RTFFI_EXIT:
	fclose(languageTable);
	fclose(languageList);
	free(listFileName);
	free(tableFileName);
	return 0;
}

int removeTweet(char *filename, long offset) {
	if(filename == NULL) return 0;

	char *dataFileName = getDataFileName(filename);
	//tries to open the file and check if it was successful
	FILE *file = fopen(dataFileName, "r+");
	if(file == NULL) return 0;

	//saving the tweet
	TWEET *removedTweet = readTweet(filename, offset);
	//getting the stack's head
	long stackHead;
	if(fread(&stackHead, HEADER, 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//tries to move til the deletion's offset
	if(fseek(file, offset, SEEK_SET) != 0) goto REMOVE_TWEET_EXIT;

	//tries to save the size of the register and moves sizeof(int) forward
	int fieldSize;
	if(fread(&fieldSize, sizeof(int), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//checks if the register is already removed
	if(fieldSize <= 0) goto REMOVE_TWEET_EXIT;

	//tries to write a long that indicates the position of the last reg removed (stack's head)
	if(fwrite(&stackHead, sizeof(long), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//tries to update the stack's head in the file's beginning
	if(fseek(file, 0, SEEK_SET) != 0) goto REMOVE_TWEET_EXIT;
	if(fwrite(&offset, sizeof(long), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//tries to back to the register and mark it as removed (negative)
	if(fseek(file, offset, SEEK_SET) != 0) goto REMOVE_TWEET_EXIT;
	fieldSize *= -1;
	if(fwrite(&fieldSize, sizeof(int), 1, file) <= 0) goto REMOVE_TWEET_EXIT;

	//removing indexes from the index files
	int langRet = removeTweetFromLanguageIndex(filename, removedTweet, offset);
	int favRet = removeTweetFromFavoriteIndex(filename, removedTweet, offset);
	if(!langRet || !favRet)
		goto REMOVE_TWEET_EXIT;

	fclose(file);
	free(dataFileName);
	return 1;

REMOVE_TWEET_EXIT:
	fclose(file);
	free(dataFileName);
	return 0;
}

void printTweet(TWEET *tweet){
	if(tweet == NULL) return;

	printf("Tweet: %s\n", 			tweet->text);
	printf("User: %s\n" , 	 		tweet->userName);
	printf("Coordinates: %s\n", 		tweet->coords);
	printf("Language: %s\n",		tweet->language);
	printf("Favorited %d time%s",tweet->favoriteCount, (tweet->favoriteCount <= 1)?"\n":"s\n");
	printf("Retweeted %d time%s",tweet->retweetCount, (tweet->retweetCount <= 1)?"\n":"s\n");
	printf("Viewed %ld time%s",tweet->viewsCount, (tweet->viewsCount <= 1)?"\n":"s\n");
	printf("_________________________________________________\n");

	return;
}

// Free the vector
void destroyTweet (TWEET **tweet){
	if(tweet == NULL || *tweet == NULL)
		return;

	free((*tweet)->text);
	free((*tweet)->userName);
	free((*tweet)->coords);
	free((*tweet)->language);

	free(*tweet);
	tweet = NULL;
}

// Get the elements that are in *v1 OR *v2
// resultSize = number of elements in default return
long *merge (long *v1, long *v2, size_t sz_v1, size_t sz_v2, long *resultSize){
	size_t iv1, iv2;	// Iterator to *v1 and *v2
	long *result, *over;	// *result = Default return // *over = vector that still have elements
	int i, end;	// i = iterator of *over // end = number of elements in *over

	iv1 = iv2 = 0;
	*resultSize = 0;

	while(iv1 < sz_v1 && iv2 < sz_v2) {	// While have elements on both vectors
		result = realloc(result, (*resultSize+1) * sizeof(long));	// Create a new position in *result

		if (v1[iv1] < v2[iv2]) {	// IF element of *v1 is the valueless
			result[*resultSize] = v1[iv1++];	// Include element in *result
		}
		else if (v2[iv2] < v1[iv1]){	// IF element of *v2 is the valueless
			result[*resultSize] = v2[iv2++];	// Include element in *result
		}
		else {	// IF element of *v1 == element of *v2
			result[*resultSize] = v1[iv1++];	// Include only one element in *result
			iv2++;
		}
		(*resultSize)++;
	}

	if (!(iv1 == sz_v1 && iv2 == sz_v2)) {	// IF one of the vectors still have elements:
		if (iv1 == sz_v1){	// IF *v1 reach the end
			over = v2;
			i = iv2;
			end = sz_v2;
		}
		else {	// IF *v2 reach the end
			over = v1;
			i = iv1;
			end = sz_v1;
		}

		// Copy the remaining elements to *result
		result = realloc(result, (*resultSize + (end - i -1)));
		for (; i < end; i++){
			result[*resultSize] = over[i];
			(*resultSize)++;
		}
	}
	return result;
}

// Get the elements that are in v1 AND v2
// resultSize = number of elements in default return
long *match (long *v1, long *v2, size_t sz_v1, size_t sz_v2, long *resultSize){
	size_t iv1, iv2;	// Iterator to *v1 and *v2
	long *result;	// Default return

	iv1 = iv2 = 0;
	*resultSize = 0;

	while(iv1 < sz_v1 && iv2 < sz_v2) {	// While have elements on both vectors
		result = realloc(result, ((*resultSize)+1) * sizeof(long));	// Create a new position in *result

		if (v1[iv1] < v2[iv2]) {	// IF element of *v1 is the valueless
			iv1++;	// Go to next element in *v1
		}
		else if (v2[iv2] < v1[iv1]){	// IF element of *v2 is the valueless
			iv2++;	// Go to next element in *v2
		}
		else {	// IF element of *v1 == element of *v2
			result[*resultSize] = v1[iv1++];	// Insert element in *result
			iv2++;	// Go to next element in *v1 and *v2
		}
		(*resultSize)++;
	}
	return result;
}
