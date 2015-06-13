#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tweet.h"

#define FNAME "tweettest"

int teste1();
int teste2();
int teste3();

int main() {
	teste1();
	teste2();
	teste3();
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
	
	
	TWEET * tt = newTweet("texto2",
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
	
	tt = newTweet("texto",
		"usuario",
		"4 8 15 S 16 23 42 O",
		"PT-BR",
		20,
		15,
		200);
	if (tt != NULL) { printf("\nTweet criado\n\n"); }
	
	printTweet(tt);
	
	writeTweet(FNAME, tt);
	printf("\n\nTweet escrito *\n");
	
	destoryTweet(&tt);
	printf("\nTweet liberado *\n");

	tt = newTweet("texto",
		"usuario",
		"4 8 15 S 16 23 42 O",
		"EN-US",
		10,
		15,
		200);
	if (tt != NULL) { printf("\nTweet criado\n\n"); }
	
	printTweet(tt);
	
	writeTweet(FNAME, tt);
	printf("\n\nTweet escrito *\n");
	
	destoryTweet(&tt);
	printf("\nTweet liberado *\n");
	
	/*
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
	*/
	
	free(str0);
	free(str1);
	free(str2);
	free(str3);
	
	return 0;
}

int teste2 (){
	long *offset, foundOccurences;
	
	// = = = = = Busca por Usuario = = = = =
	foundOccurences = 0;
	offset = NULL;
	printf("\nProcurando : findOffsetByUser : \"usuario\"\n");
	offset = findOffsetByUser(FNAME, "usuario", &foundOccurences);
	printf("achou usuario\n");
	printVector(offset, foundOccurences);
	free(offset);

	printf("\nProcurando : findOffsetByUser : \"usuario%c\"\n", END_FIELD);
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
	printf("\nProcurando : findDataOffsetByFavoriteCount : 10\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findDataOffsetByFavoriteCount(FNAME, 10, &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findDataOffsetByFavoriteCount : 300 (Falha)\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findDataOffsetByFavoriteCount(FNAME, 300, &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	// = = = = = Busca por Language = = = = =
	printf("\nProcurando : findDataOffsetByLanguage : \"PT-BR\"\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findDataOffsetByLanguage(FNAME, "PT-BR", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findDataOffsetByLanguage : \"PT-BR\\31\"(Falha)\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findDataOffsetByLanguage(FNAME, "PT-BR\31", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	
	printf("\nProcurando : findDataOffsetByLanguage : \"EN-US\"\n");
	foundOccurences = 0;
	offset = NULL;
	offset = findDataOffsetByLanguage(FNAME, "EN-US", &foundOccurences);
	printVector(offset, foundOccurences);
	free(offset);
	return 0;
}

int teste3(){

	long *offset;
	long foundOccurences = 0;
	offset = NULL;
	printf("\nProcurando : findOffsetByUser : \"usuario\"\n");
	offset = findOffsetByUser(FNAME, "usuario", &foundOccurences);
	printf("achou usuario\n");
	removeTweet(FNAME, offset[0]);
	free(offset);

	TWEET *tt = newTweet("esse texto e muito grande para caber no tamanho que foi removido, acho que vai zoar o negocio",
		"usuario",
		"4 S 6 N",
		"PT-BR",
		10,
		15,
		30);
	if (tt != NULL) { printf("\nTweet criado\n\n"); }
	
	printTweet(tt);
	
	writeTweet(FNAME, tt);
	printf("\n\nTweet escrito *\n");
	
	destoryTweet(&tt);
	printf("\nTweet liberado *\n");

	foundOccurences = 0;
	offset = NULL;
	printf("\nProcurando : language : \"EN-US\"\n");
	offset = findDataOffsetByLanguage(FNAME, "EN-US", &foundOccurences);
	printf("achou usuario\n");
	removeTweet(FNAME, offset[0]);
	free(offset);


}
