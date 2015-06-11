#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define HEADER (sizeof(long))	// Size of the data before the first tweet
#define minTweetSize  (sizeof(int) + 4 + 2*sizeof(int) + sizeof(long))
#define INDEXHEADER (sizeof(int)) // size of the date before the keys
#define UPDATED 1
#define MAX_LANGUAGE_SIZE 50

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

	for (lim = nmemb; lim != 0; lim >>= 1) {
		int index = (lim >> 1) * size;
		p = base + index;
		cmp = (*compar)(key, p);
		if (cmp == 0)
			return index;
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

typedef struct langIndexItem{

	char language[MAX_LANGUAGE_SIZE];
	long byteOffset;

}LANGITEM;

typedef struct langIndexListItem{
	
	long fileOffset;
	long next;

}LANGLISTITEM;

static char *getLanguageTableIndexFileName(char *filename);
static char *getLanguageListIndexFileName(char *filename);
static char *getFavoriteTableIndexFileName(char *filename);
static char *getFavoriteListIndexFileName(char *filename);

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

	while(stackPos != -1 && minDifference > 0){
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

static void updateLanguageIndexFiles(char* table, char* list, TWEET *tweet, long byteOffset){
	
	FILE *indexTable = fopen(table, "r+");
	FILE *indexList;
	int updatedStatus = !UPDATED;

	fseek(indexTable, 0, SEEK_END);
	long fileSize = ftell(indexTable);
	LANGITEM *langVector = malloc(fileSize - INDEXHEADER);
	LANGITEM newEntryOnTab;	
	LANGLISTITEM newEntryOnList;
	int size = (fileSize - sizeof(int))/sizeof(LANGITEM); 
	
	int len = strlen(tweet->language);
	strncpy(newEntryOnTab.language, tweet->language, (MAX_LANGUAGE_SIZE < len)?MAX_LANGUAGE_SIZE:len);
	newEntryOnTab.byteOffset = byteOffset;
	newEntryOnTab.language[MAX_LANGUAGE_SIZE - 1] = 0;	

	fread(langVector, sizeof(LANGITEM), size, indexTable);

	int index = binarySearch(&newEntryOnTab, langVector, size, sizeof(LANGITEM), compareLanguageItem);	
	int found = index;

	if(found == -1){
		langVector = realloc(langVector, (fileSize - sizeof(int) + sizeof(LANGITEM))); //adicionado um espaço para acrescentar a nova entrada
		langVector[size - 1].byteOffset = -1; //insiro a variavel de insercao no final do vetor
		strcpy(langVector[size - 1].language, newEntryOnTab.language);
		index = size - 1;
	}

	indexList = fopen(list, "r+"); //abro a lista em modo append
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);
	fseek(indexList, 0, SEEK_END);

	newEntryOnTab.byteOffset = ftell(indexList);//salvo o byteoffset na variavel que vai para a tab
	newEntryOnList.fileOffset = byteOffset;
	newEntryOnList.next = langVector[index].byteOffset;
	
	fwrite(&newEntryOnList, sizeof newEntryOnList, 1, indexList);
	langVector[index].byteOffset = newEntryOnTab.byteOffset;

	if(found == -1)
		qsort(langVector,size, sizeof(LANGITEM), compareLanguageItem);//ordeno o vetor
	
	updatedStatus = UPDATED;	
	fclose(indexTable);
	indexTable = fopen(table, "w+");
	fwrite(&updatedStatus, sizeof(int), 1, indexTable);	
	fwrite(langVector, sizeof(LANGITEM), size, indexTable);

	fseek(indexList, 0, SEEK_SET);
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);

	fclose(indexTable);
	fclose(indexList);
	free(langVector);
}

static void updateFavoriteCountIndexFiles(char* table, char* list, TWEET *tweet, long byteOffset){
/*
	FILE *indexTable = fopen(table, "r+");
	FILE *indexList;
	int updatedStatus = !UPDATED;

	fseek(indexTable, 0, SEEK_END);
	long fileSize = ftell(indexTable);
	FAVITEM *favVector = malloc(fileSize - INDEXHEADER);
	FAVITEM newEntryOnTab;	
	LANGLISTITEM newEntryOnList;
	int size = (fileSize - sizeof(int))/sizeof(LANGITEM); 
	
	int len = strlen(tweet->language);
	strncpy(newEntryOnTab.language, tweet->language, (MAX_LANGUAGE_SIZE < len)?MAX_LANGUAGE_SIZE:len);
	newEntryOnTab.byteOffset = byteOffset;
	newEntryOnTab.language[MAX_LANGUAGE_SIZE - 1] = 0;	

	fread(langVector, sizeof(LANGITEM), size, indexTable);

	int index = binarySearch(&newEntryOnTab, langVector, size, sizeof(LANGITEM), compareLanguageItem);	
	int found = index;

	if(found == -1){
		langVector = realloc(langVector, (fileSize - sizeof(int) + sizeof(LANGITEM))); //adicionado um espaço para acrescentar a nova entrada
		langVector[size - 1].byteOffset = -1; //insiro a variavel de insercao no final do vetor
		strcpy(langVector[size - 1].language, newEntryOnTab.language);
		index = size - 1;
	}

	indexList = fopen(list, "r+"); //abro a lista em modo append
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);
	fseek(indexList, 0, SEEK_END);

	newEntryOnTab.byteOffset = ftell(indexList);//salvo o byteoffset na variavel que vai para a tab
	newEntryOnList.fileOffset = byteOffset;
	newEntryOnList.next = langVector[index].byteOffset;
	
	fwrite(&newEntryOnList, sizeof newEntryOnList, 1, indexList);
	langVector[index].byteOffset = newEntryOnTab.byteOffset;

	if(found == -1)
		qsort(langVector,size, sizeof(LANGITEM), compareLanguageItem);//ordeno o vetor
	
	updatedStatus = UPDATED;	
	fclose(indexTable);
	indexTable = fopen(table, "w+");
	fwrite(&updatedStatus, sizeof(int), 1, indexTable);	
	fwrite(langVector, sizeof(LANGITEM), size, indexTable);

	fseek(indexList, 0, SEEK_SET);
	fwrite(&updatedStatus, sizeof updatedStatus, 1, indexList);

	fclose(indexTable);
	fclose(indexList);
	free(langVector);
	*/
}

static void updateIndexFiles(TWEET *tweet, long byteOffset, char* filename){
	
	char *table = getLanguageTableIndexFileName(filename);
	char *list = getLanguageListIndexFileName(filename);
	
	updateLanguageIndexFiles (table, list, tweet, byteOffset);

	table = getFavoriteTableIndexFileName(filename);
	list = getFavoriteListIndexFileName(filename);



}

void writeTweet(char *filename, TWEET *tweet){
	if(filename == NULL || tweet == NULL) return;

	long tweetsize = tweetSize(tweet);  //tamanho do tweet a ser inserido
	long previusOffset, previusOffsetValue, byteOffset;
	
	char *datafilename = getDataFileName(filename); //nome do arquivo de dados
	FILE *arq = fopen(datafilename, "w+");
	if(arq == NULL) return;

	long stackHead;
	if(fread(&stackHead, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT; //cabeça da pilha de removidos
	
	if(stackHead == -1){ // não existe espaço logicamente removido
		if(fseek(arq, 0, SEEK_END) != 0) goto WRITETWEET_EXIT;
		flushTweet(datafilename, ftell(arq), tweetsize, tweet);

	}else{//existe espaço logicamente removido
		byteOffset = bestFit(datafilename, tweetsize, &previusOffset); //byteoffset para a inserção do tweet
		int fieldSize;
		
		if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT; //posicionado o cursor para o inicio do offset de insercao
		if(fread(&fieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT; //lendo o tamanho do campo de insersao
		long newFieldSize = fieldSize - (tweetsize + sizeof(int));
		
		if(newFieldSize < minTweetSize){ //o tamanho do campo de insercao eh muito pequeno
			long nextOffset;
			if(fseek(arq, byteOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT; //cursor antes do long que indica o proximo campo removido 
			if(fread(&nextOffset, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT; //lendo a posicao do proximo campo removido
			if(fseek(arq, previusOffset + sizeof(int), SEEK_SET) != 0) goto WRITETWEET_EXIT; //movendo cursor para o campo anterior
			if(fwrite(&nextOffset, sizeof(long), 1, arq) <= 0) goto WRITETWEET_EXIT;//escrevendo a proxima posicao no campo anterior
		}else{//o campo eh muito pequeno
			byteOffset += sizeof(int) + newFieldSize;//atualizando a variavel de posicao do tweet a ser inserido, deixando um espaco removido antes
		}

		newFieldSize *= -1;//atualizando o tamanho do campo novo para removido (negativo)			
		if(fseek(arq, byteOffset, SEEK_SET) != 0) goto WRITETWEET_EXIT;//posicionando o cursor para o comeco do campo 
		if(fwrite(&newFieldSize, sizeof(int), 1, arq) <= 0) goto WRITETWEET_EXIT;//escrevendo o novo valor do campo
		flushTweet(datafilename, byteOffset, tweetsize, tweet);//inserindo tweet 
	}
	//atualizando os indices
	updateIndexFiles(tweet, byteOffset, filename);

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

static char *getFavoriteTableIndexFileName(char *filename){/*{*/
	char *file;
	file = getDataFileName(filename);
	strcpy(&(file[strlen(file)-4]), ".itf");
	return file;
}/*}*/

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
	FILE *favIdxTabFile = fopen(dataFileName, "w+");
	fclose(favIdxTabFile);
	free(favIdxTabFileName);
	char *favIdxListFileName = getFavoriteListIndexFileName(filename);
	FILE *favIdxListFile = fopen(dataFileName, "w+");
	fclose(favIdxListFile);
	free(favIdxListFileName);
	char *langIdxTabFileName = getLanguageTableIndexFileName(filename);
	FILE *langIdxTabFile = fopen(dataFileName, "w+");
	fclose(langIdxTabFile);
	free(langIdxTabFileName);
	char *langIdxListFileName = getLanguageListIndexFileName(filename);
	FILE *langIdxListFile = fopen(dataFileName, "w+");
	fclose(langIdxListFile);
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

generateIdxData:
	fclose(dataFile);

}

static long findIndexOffsetByFavoriteCount(char *filename, int favoriteCount){
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
	FAVITEM *items = malloc(nTweets*sizeof(FAVITEM));
	if (items == NULL)
		goto findFavRetTab;
	if (fread(items, sizeof(FAVITEM), nTweets, fileTab) <= 0)
		goto findFavRetTab;

	FAVITEM key;
	key.favoriteCount = favoriteCount;

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
	long ret = NULL;
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
	LANGITEM *items = malloc(nTweets*sizeof(LANGITEM));
	if (items == NULL)
		goto findLangRetTab;
	if (fread(items, sizeof(LANGITEM), nTweets, fileTab) <= 0)
		goto findLangRetTab;

	LANGITEM key;
	int len = strlen(language);
	strncpy(key.language, language, (MAX_LANGUAGE_SIZE < len)?MAX_LANGUAGE_SIZE:len);
	key.language[MAX_LANGUAGE_SIZE-1] = 0;

	int found = binarySearch(&key, items, nTweets, sizeof(LANGITEM), compareLanguageItem);
	if (found == -1)
		goto findLangRetTab;

	ret = (found*sizeof(LANGITEM))+INDEXHEADER;

findLangRetTab:
	fclose(fileTab);
findLangRet:
	return ret;
}

long *findDataOffsetByFavoriteCount(char *filename, int favoriteCount, long *foundOccurences){
	*foundOccurences = 0;
	long *ret = NULL;
	char *listFileName = getFavoriteListIndexFileName(filename);
	FILE *fileList = fopen(listFileName, "r");
	free(listFileName);
	char *tabFileName = getFavoriteTableIndexFileName(filename);
	FILE *fileTab = fopen(tabFileName, "r");
	free(tabFileName);

	long offset;
	offset = findIndexOffsetByFavoriteCount(filename, favoriteCount);
	fseek(fileTab, offset, SEEK_SET);
	FAVITEM i;
	fread(&i, sizeof i, 1, fileTab);
	fseek(fileList, i.byteOffset, SEEK_SET);

	FAVLISTITEM current;
	do {
		if (fread(&current, sizeof current, 1, fileList) == 0)
			goto findFavRetList;
		*foundOccurences++;
		ret = realloc(ret, (sizeof current)*(*foundOccurences));
		if (ret == NULL)
			goto findFavRetList;
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

	long offset;
	offset = findIndexOffsetByLanguage(filename, language);
	fseek(fileTab, offset, SEEK_SET);
	FAVITEM i;
	fread(&i, sizeof i, 1, fileTab);
	fseek(fileList, i.byteOffset, SEEK_SET);

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

static int removeTweetFromFavoriteIndex(char *filename, TWEET *removedTweet, long offset) {
	char *tableFileName = getFavoriteTableIndexFileName(filename);
	char *listFileName  = getFavoriteListIndexFileName(filename);
	FILE *favoriteTable = fopen(tableFileName, "r+");
	FILE *favoriteList  = fopen(listFileName,  "r+");
	if(favoriteTable == NULL || favoriteList == NULL || removedTweet == NULL) return 0;

	//marking table status as not updated
	int tableStatus = !UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
	//getting the offset of the tweet in the table index file
	long tableOffset = findIndexOffsetByFavoriteCount(filename, removedTweet->favoriteCount);
	//getting the byteOffset for the item in the list index file
	FAVITEM *tableItem;
	if(fseek(favoriteTable, tableOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(tableItem, sizeof(FAVITEM), 1, favoriteTable) <= 0) goto RTFFI_EXIT;
	//going to the list index file 
	FAVLISTITEM *current, *previous;
	if(fseek(favoriteList, tableItem->byteOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	if(fread(current, sizeof(FAVLISTITEM), 1, favoriteTable) <= 0)
		goto RTFFI_EXIT;
	//if the next in the list doesn't exist and the byteoffset is the same as the tweet we're removing
	if(current->next == -1 && current->fileOffset == offset) {
		if(fseek(favoriteTable, SEEK_END, SEEK_SET) != 0) goto RTFFI_EXIT;
		long sizeOfTableFile = ftell(favoriteTable);
		sizeOfTableFile-= INDEXHEADER;
		//we load the table index file to the memory
		FAVITEM *aux = (FAVITEM *) malloc(sizeOfTableFile);
		if (aux == NULL) goto RTFFI_EXIT;
		//swap the last item in the list with the one to be removed
		aux[(tableOffset - INDEXHEADER)/(sizeOfTableFile/sizeof(FAVITEM))] = aux[(sizeOfTableFile/sizeof(FAVITEM))-1];
		//realloc the size of the list, cropping the deleted item out
		aux = realloc(aux, sizeOfTableFile - sizeof(FAVITEM));
		//sort it again
		qsort(aux, (sizeOfTableFile/sizeof(FAVITEM)), sizeof(FAVITEM), compareFavoriteItem);
		//and write it back to the file
		if(fwrite(aux, sizeOfTableFile, 1, favoriteTable)) goto RTFFI_EXIT;
		//as we've finished removing the element, we update the table status to updated and return
		rewind(favoriteTable);
		tableStatus = UPDATED;
		if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
		free(aux);
		return 1;
	}
	//finding the item in the list file and removing it
	do {
		previous = current;
		if(fread(current, sizeof(FAVLISTITEM), 1, favoriteTable) <= 0)
			goto RTFFI_EXIT;
		if(fseek(favoriteList, current->fileOffset, SEEK_SET) != 0) goto RTFFI_EXIT;
	} while(current->fileOffset != offset || current->next != -1);

	previous->next = current->next;
	//setting the table status to updated and returning
	rewind(favoriteTable);
	tableStatus = UPDATED;
	if(fwrite(&tableStatus, INDEXHEADER, 1, favoriteTable) <= 0) goto RTFFI_EXIT;
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

	long tableOffset = findIndexOffsetByLanguage(filename, removedTweet->language);

	FAVITEM *tableItem;
	if(fseek(languageTable, tableOffset, SEEK_SET) != 0) goto RTFLI_EXIT;
	if(fread(tableItem, sizeof(FAVITEM), 1, languageTable) <= 0) goto RTFLI_EXIT;

	FAVLISTITEM *current, *previous;
	if(fseek(languageList, tableItem->byteOffset, SEEK_SET) != 0) goto RTFLI_EXIT;
	if(fread(current, sizeof(FAVLISTITEM), 1, languageTable) <= 0)
		goto RTFLI_EXIT;

	if(current->next == -1 && current->fileOffset == offset) {
		if(fseek(languageTable, SEEK_END, SEEK_SET) != 0) goto RTFLI_EXIT;
		long sizeOfTableFile = ftell(languageTable);
		sizeOfTableFile-= INDEXHEADER;

		FAVITEM *aux = (FAVITEM *) malloc(sizeOfTableFile);
		if (aux == NULL) goto RTFLI_EXIT;
		aux[(tableOffset - INDEXHEADER)/(sizeOfTableFile/sizeof(FAVITEM))] = aux[(sizeOfTableFile/sizeof(FAVITEM))-1];
		aux = realloc(aux, sizeOfTableFile - sizeof(FAVITEM));
		qsort(aux, (sizeOfTableFile/sizeof(FAVITEM)), sizeof(FAVITEM), compareLanguageItem);
		if(fwrite(aux, sizeOfTableFile, 1, languageTable)) goto RTFLI_EXIT;

		rewind(languageTable);
		tableStatus = UPDATED;
		if(fwrite(&tableStatus, INDEXHEADER, 1, languageTable) <= 0) goto RTFLI_EXIT;
		free(aux);
		return 1;
	}

	do {
		previous = current;
		if(fread(current, sizeof(FAVLISTITEM), 1, languageTable) <= 0)
			goto RTFLI_EXIT;
		if(fseek(languageList, current->fileOffset, SEEK_SET) != 0) goto RTFLI_EXIT;
	} while(current->fileOffset != offset || current->next != -1);

	previous->next = current->next;
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

	//removing indexes from the indexes fieldSize
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
	printf("Favorited %d time%s",tweet->favoriteCount, (tweet->favoriteCount <= 1)?"\n":"s\n");
	printf("Retweeted %d time%s",tweet->retweetCount, (tweet->retweetCount <= 1)?"\n":"s\n");
	printf("Viewed %ld time%s",tweet->viewsCount, (tweet->viewsCount <= 1)?"\n":"s\n");
	printf("_________________________________________________\n");

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
	tweet = NULL;
}
