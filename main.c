#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define FNAME "tweettest"

int teste1();
int teste2();

int main() {
	teste1();
	teste2();
	return 0;
}

void printVector(long * vect, int n){
	if (vect == NULL || &n == NULL || n <= 0){
		printf("\tVetor Vazio\n");
		return;
	}
	
	int i;
	for (i = 0; i < n; i++){
		printf("\t> %ld\n", vect[i]);
	}
}

int teste1 (){
	int foundOccurences, i;
	char * str0 = calloc(sizeof(char), 14);
	char * str1 = calloc(sizeof(char), 3);
	char * str2 = calloc(sizeof(char), 14);
	char * str3 = calloc(sizeof(char), 2);
	
	
	TWEET * tt = newTweet("texto",
		"usuario",
		"4 8 15 S 16 23 42 O",
		"PT-BR",
		10,
		15,
		200);
	if (tt != NULL) { printf("\nTweet criado\n\n"); }
	
	printTweet(tt);
	
	writeTweet(FNAME, tt);
	printf("\n\nTweet escrito *\n");
	
	destoryTweet(&tt);
	printf("\nTweet liberado *\n");
	
	tt = readTweet(FNAME, sizeof(long));
	if (tt != NULL) { printf("\nTweet lido\n"); }
	
	printTweet(tt);
	destoryTweet(&tt);
	
	for (i = 0; i < 50; i++){
		printf("-- FOR %2d |", i);
		sprintf(str0, "%d", rand());
		sprintf(str1, "%d", (i%7)*10);
		sprintf(str2, "%d", rand());
		sprintf(str3, "%d", (i%3));
		
		printf("\tAleatório | ");
		
		tt = newTweet(str0, str1, str2, str3, rand()%30, rand()%30, rand()%200);
		printf("Novo Tweet | ");
		
		writeTweet(FNAME, tt);
		printf("Escreveu | ");
		
		destoryTweet(&tt);
		printf("Destruiu\n");
	}
	
	free(str0);
	free(str1);
	free(str2);
	free(str3);
	
	return 0;
}

int teste2 (){
	long *offset, foundOccurences;
	
	// = = = = = Busca por Usuario = = = = =
	printf("\nProcurando : findOffsetByUser : \"usuario\"\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByUser(FNAME, "usuario", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findOffsetByUser : \"usuario\\31\"\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByUser(FNAME, "usuario\31", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findOffsetByUser : \"u4s8u15a16r23i42o\\31\" (DEVE falhar SEM segfault)\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByUser(FNAME, "u4s8u15a16r23i42o\31", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	// = = = = = Busca por Favoritos = = = = =
	printf("\nProcurando : findOffsetByFavoriteCount : 10\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByFavoriteCount(FNAME, 10, &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findOffsetByFavoriteCount : 300 (Falha)\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByFavoriteCount(FNAME, 300, &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	// = = = = = Busca por Language = = = = =
	printf("\nProcurando : findOffsetByLanguage : \"PT-BR\"\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByLanguage(FNAME, "PT-BR", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findOffsetByLanguage : \"PT-BR\\31\"\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByLanguage(FNAME, "PT-BR\31", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findOffsetByLanguage : \"EN-BR\\31\" (Falha)\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findOffsetByLanguage(FNAME, "EN-BR\31", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	return 0;
}