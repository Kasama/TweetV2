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

#include "tweet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Useful defines
#define CMD_LENGTH 30

#define CMD_INSERT "insert"
#define CMD_EXIT "exit"
#define CMD_REMOVE "remove"
#define CMD_REQUEST "request"
#define CMD_REQUEST_ALL "all"
#define CMD_REQUEST_FAVORITE "favorite"
#define CMD_REQUEST_LANGUAGE "language"
#define CMD_REQUEST_MERGE "merge"
#define CMD_REQUEST_MATCH "match"
#define CMD_REQUEST_USER "user"
#define CMD_CREDITS "credits"
#define CMD_LS "ls"
#define CMD_CD "cd"
#define CMD_PWD "pwd"
#define CMD_MKDIR "mkdir"
#define CMD_CHANGE_DATAFILE "cf"
#define CMD_CLEAR "clear"
#define CMD_HELP "help"

#define SPEC_PDF "http://wiki.icmc.usp.br/images/b/bd/SCC0215012015projeto02TurmaBa.pdf"
#define DOC_PDF "https://www.github.com/Kasama/TweetV2/"
#define PROFESSOR_NAME "Cristina Dutra de Aguiar Ciferri"

// OS dependent functions
#ifdef _WIN32
#define CLEAR "cls"
#define LS "dir"
#define SEP "\\"
#define CHANGEDIR(X) _chdir(X)
#define WORKINGDIRECTORY(X, Y) _getcwd(X, Y)
#define MKDIRECTORY(X, Y) _mkdir(X)
#include <direct.h>
#else
#define CLEAR "clear"
#define LS "ls"
#define SEP "/"
#define CHANGEDIR(X) chdir(X)
#define WORKINGDIRECTORY(X, Y) getcwd(X, Y)
#define MKDIRECTORY(X, Y) mkdir(X, Y)
#include <unistd.h>
#include <sys/stat.h>
#endif

enum {

	PROGNAME,	//0
	FILENAME	//1

};

char *readField(FILE *stream, char end) {
	char *buffer = NULL;
	char character;
	int counter = 0;

	do {
		character = fgetc(stream);
		buffer = (char *) realloc(buffer, sizeof(char)
				* (counter+1));
		buffer[counter++] = character;
	}while (character != end);
	buffer[counter-1] = '\0';

	return buffer;
}
/**
 * A function that recieves from the user the information to add to the file and calls
 * the function insertTweet, from the library implementation
 *
 * @param fileName - the name of the currently in use file
 *
 * @variable user - the userName to be inserted
 * @variable lang - the language to be inserted
 * @variable coords - the coordinates to be inserted
 * @variable text - the text to be inserted
 * @variable fav - the favorite count to be inserted
 * @variable retweet - the retweet count to be inserted
 * @variable view - the number of views to be inserted
 * @variable ret - stores the return value of the insert function for further use
 *
 */
void cmdInsert(char *fileName){
	char *user;
	char *lang;
	char *coords;
	char *text;
	int fav, retweet, ret;
	long view;

	//request all information needed to insert a new tweet
	printf("Please type a user name:\n");
	scanf("\n"); //read a \n that is still in the buffer from the previous scanf
	user = readField(stdin, '\n');
	printf("Type your geographic coordinates:\n");
	coords = readField(stdin, '\n');
	printf("Specify the language of your text:\n");
	lang = readField(stdin, '\n');
	printf("Type your text:\n");
	text = readField(stdin, '\n');
	printf("Type how many favorites your tweet has:\n");
	scanf("%d", &fav);
	printf("Type how many views your tweet has:\n");
	scanf("%ld", &view);
	printf("Type how many retweets your tweet has:\n");
	scanf("%d", &retweet);

	TWEET* tweet = newTweet(text, user, coords, lang, fav, view, retweet);

	// call the function that will insert the tweet into the database
	ret = writeTweet(fileName, tweet);

	// check the return value and print a error message if needed
	if(ret == 1){
		printf("Tweet inserted successfully!\n");
	}else{
		printf("Could not insert the tweet. Is the filesystem read only?\n");
	}
	free(text);
	free(user);
	free(coords);
	free(lang);
	destroyTweet(&tweet);
}

/**
 * A function that gets the RRN and calls the removeTweet function
 * to remove a tweet
 *
 * @param fileName - the name of the currently in use file
 *
 * @variable RRN - the relative register number (ID) to be removed from the database
 * @variable ret - stores the return value of the removal function for further use
 *
 */
void cmdRemove(char *fileName){
	int favorites, ret, choice;
	long *offsets, amount;

	scanf("%d", &favorites);

	offsets = findDataOffsetByFavoriteCount(fileName, favorites, &amount);

	for (int i = 0; i < amount; i++){
		printf("Tweet %d\n", i+1);
		TWEET *tt;
		tt = readTweet(fileName, offsets[i]);
		printTweet(tt);
		destroyTweet(&tt);
		if (i+1 < amount){
				printf("------------------- Press enter to see next tweet\n"); 
			getchar();
		}
	}
	printf("select the number of the tweet that you want to remove: ");
	scanf("%d", &choice);
	choice--;
	if (choice < 0 || choice >= amount){
		printf("invalid number!\n");
		return;
	}
	printf("removing...\n");
	TWEET *tt;
	tt = readTweet(fileName, offsets[choice]);
	printTweet(tt);
	destroyTweet(&tt);

	ret = removeTweet(fileName, offsets[choice]);
	// check the return value and print a error message if needed
	if(ret == 1){
		printf("The Tweet %d was successfully removed\n", choice+1);
	}else{
		printf("The Tweet %d could not be removed. Maybe it does not exist\n", choice+1);
	}
}

/**
 * A function that requests tweet data and print to screen, depending on which tweets the user
 * requests (all, one or all from a single user)
 *
 * @param fileName - the name of the currently in use file
 *
 * @variable RRN - the relative register number (ID) to be requested from the database
 * @variable tweets - a array of tweets returned from the requestAll and findTweetByUser functions
 * @variable tweet - a single tweet returned from the requestTweet function
 * @variable cmd - the subcommand typed after the "request" command
 * @variable buf - a buffer, to consume the stdin buffer in case of a invalid command
 * @variable RRN - the RRN to use as parameter to the requestTweet function
 * @variable ammount - the ammount of tweets returned by the requestAll or findTweetByUser functions
 * @variable i - a dummy counter
 *
 */
void cmdRequest(char *fileName){
	char cmd[CMD_LENGTH], buf[10*CMD_LENGTH];
	int favorite;
	char *language;
	long amount = 0;
	long *offsets = NULL;

	// reads the next part of the request command
	scanf("%s", cmd); 
	// a "switch" structure 
	if(strcmp(cmd, CMD_REQUEST_ALL) == 0){
		offsets = findAllTweets(fileName, &amount);
	}else if(strcmp(cmd, CMD_REQUEST_USER) == 0){
		scanf("%s", cmd); // reads the user to find for
		offsets = findOffsetByUser(fileName, cmd, &amount);
	}else if(strcmp(cmd, CMD_REQUEST_FAVORITE) == 0){
		scanf("%d", &favorite); // reads the RRN to request from
		offsets = findDataOffsetByFavoriteCount(fileName, favorite, &amount);
	}else if(strcmp(cmd, CMD_REQUEST_LANGUAGE) == 0){
		scanf(" ");
		language = readField(stdin, '\n');
		offsets = findDataOffsetByLanguage(fileName, language, &amount);
	}else if(strcmp(cmd, CMD_REQUEST_MERGE) == 0){
		scanf("%d", &favorite);
		scanf(" ");
		language = readField(stdin, '\n');
		long *favOffsets = findDataOffsetByFavoriteCount(fileName, favorite, &amount);
		long favAmount = amount;
		long *langOffsets = findDataOffsetByLanguage(fileName, language, &amount);
		long langAmount = amount;
		offsets = merge(favOffsets, langOffsets, favAmount, langAmount, &amount);
	}else if(strcmp(cmd, CMD_REQUEST_MATCH) == 0){
		scanf("%d", &favorite);
		scanf(" ");
		language = readField(stdin, '\n');
		long *favOffsets = findDataOffsetByFavoriteCount(fileName, favorite, &amount);
		long favAmount = amount;
		long *langOffsets = findDataOffsetByLanguage(fileName, language, &amount);
		long langAmount = amount;
		offsets = match(favOffsets, langOffsets, favAmount, langAmount, &amount);
	}else{
		fgets(buf, sizeof buf, stdin); // if the command is invalid, consume the stdin buffer
		buf[strlen(buf)-1] = 0; // as fgets adds the \n to the buffer, we remove it
		printf("Invalid command: %s%s, try typing '%s' for help\n", cmd, buf, CMD_HELP); // says that the command is not valid
	}

	if(offsets == NULL){ // if no tweets were found, print a error message
		printf("Could not find requested Tweet(s). Maybe it does not exist\n");
	}else{
		getchar();
		for (int i = 0; i < amount; i++){
			TWEET *tt;
			tt = readTweet(fileName, offsets[i]);
			printTweet(tt);
			destroyTweet(&tt);
			if (i+1 < amount){
				printf("------------------- Press enter to see next tweet\n"); 
				getchar();
			}
		}
	}
}

/**
 * A function that print this application credits
 */
void cmdCredits(){
	printf("---------------------------------------------------\n");
	printf("This program was made by:\n\
\n\
* Carlos Alberto Schneider Junior - 9167910\n\
* Frederico de Azevedo Marques    - 8936926\n\
* Henrique Martins Loschiavo      - 8936972\n\
* Lucas Kassouf Crocomo           - 8937420\n\
* Roberto Pommella Alegro         - 8936756\n\
\n\
Using the specification available at:\n\
%s\n\
and the knowledge obtained from the classes of professor %s\n\
---------------------------------------------------\n", SPEC_PDF, PROFESSOR_NAME);
}

/**
 * A function that provides a help chart for this application
 *
 * @param progname - the name of this application, as it was executed
 *
 */
void cmdHelp(char *progname){
	printf("---------------------------------------------------\n");
	printf("Usage: %s dataBaseName\n", progname);
	printf("\n"); 
	printf("%s\t\t\t- print this help\n", CMD_HELP);
	printf("%s\t\t\t- print the credits\n", CMD_CREDITS);
	printf("%s\t\t\t- insert a tweet into the database (You will be prompted asking for the information to store)\n", CMD_INSERT);
	printf("%s <Favorites>\t\t- remove a tweet from the database using the number of favorites it has\n", CMD_REMOVE);
	printf("%s %s\t\t- print all tweets in the database\n", CMD_REQUEST, CMD_REQUEST_ALL);
	printf("%s %s <UserName>\t- print all tweets of the user <UserName>\n", CMD_REQUEST, CMD_REQUEST_USER);
	printf("%s %s <Favorites>\t- print all tweets that have the specified number of favorites\n", CMD_REQUEST, CMD_REQUEST_FAVORITE);
	printf("%s %s <Language>\t- print all tweets that have the specified language\n", CMD_REQUEST, CMD_REQUEST_LANGUAGE);
	printf("%s %s <Favorites> <Language>\t- print all tweets that have the specified number of favorites AND the specified language\n", CMD_REQUEST, CMD_REQUEST_MATCH);
	printf("%s %s <Favorites> <Language>\t- print all tweets that have the specified number of favorites OR the specified language\n", CMD_REQUEST, CMD_REQUEST_MERGE);
	printf("%s <fileName>\t\t- change the data file to the specified one\n", CMD_CHANGE_DATAFILE);
	printf("%s\t\t\t- list all files in current directory\n", CMD_LS);
	printf("%s\t\t\t- change the current directory\n", CMD_CD);
	printf("%s <path>\t\t- create a new directory\n", CMD_MKDIR);
	printf("%s\t\t\t- print the path of the current directory\n", CMD_PWD);
	printf("%s\t\t\t- clear the screen \n", CMD_CLEAR);
	printf("\n");
	printf("Further documentation can be found in the public repository at: \n");
	printf("%s\n", DOC_PDF);
	printf("---------------------------------------------------\n");

}

void cmdMkDir(){
	char path[FILENAME_MAX];
	scanf("%s", path);
	MKDIRECTORY(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

}

/**
 * A function that changes the currently in use file
 *
 * @param fileName - the name of the currently in use file
 *
 */
void cmdCF(char *fileName){
	char fileN[FILENAME_MAX];
	FILE *file;
	printf("Currently open file: %s\n", fileName); 
	printf("Type a new filename to open or press enter to keep current one (it will be created if it does not exist):\n");
	fileN[0] = 0;
	WORKINGDIRECTORY(fileN, sizeof fileN);
	strcat(fileN, SEP);
	scanf("%s", &fileN[strlen(fileN)]);
	file = fopen(fileName, "a+");
	if (file == NULL){
		fprintf(stderr, "A problem was encountered when trying to open the file, will keep current one. Maybe check your permissions\n");
	}else{
		fclose(file);
		if (strcmp(fileN, "") != 0){
			strcpy(fileName, fileN);
		}
	}
}


/**
 * A function that prints the current working directory of the running application
 */
void cmdPWD(){
	char buf[FILENAME_MAX];
	printf("%s\n",WORKINGDIRECTORY(buf, sizeof buf));
}

/**
 * A function that changes the current working directory of the running application
 */
void cmdCd(){
	
	char path[FILENAME_MAX];
	scanf("%s", path);

	CHANGEDIR(path);
}

/**
 * A function that calls the OS file listing application
 */
void cmdLs(){
	system(LS);
}

/**
 * A function that calls the OS screen cleaning tool
 */
void cmdClear(){
	system(CLEAR);
}

/**
 * The main application function
 *
 * @param argc - the number of command line arguments
 * @param argv - the command line arguments
 *
 * @variable cmd - the command typed by the user
 * @variable buf - a temporary buffer for general use
 * @variable fileName - contains the name of the currently in use file
 * @variable file - used only to "touch" the file with fileName and check if its possible to open/create a file
 *
 */
int main(int argc, char *argv[]){

	char cmd[CMD_LENGTH], buf[CMD_LENGTH * 10];
	char fileName[FILENAME_MAX];
	FILE *file;

	fileName[0] = 0;
	strcat(fileName, WORKINGDIRECTORY(buf, sizeof buf));
	strcat(fileName, SEP);
	// if it was not passed as a command line argument, ask for the name of the file to work with
	if (argc != 2){
		printf("please input the name of a database to work with. files will be created if they do not exist:\n");
		scanf("%s", buf);
	}else{ // if it was, just put it in the fileName variable
		strcpy(buf, argv[FILENAME]);
	}
	strcat(fileName, buf);
	file = fopen(fileName, "a+"); // try to open the file for appending, creating it if needed (used to check if its possible to read/create files)
	if (file == NULL){ // if its not possible, print an error and exit
		fprintf(stderr, "A problem was encountered when trying to open a file, maybe check your permissions\n");
		exit(1);
	}
	fclose(file); // close the file, as we don't need it anymore

	printf("~(%s)~> ", WORKINGDIRECTORY(buf, sizeof buf)); // a simple prompt with current working directory to wait for a command

	// read commands until EOF is reached (or the command typed is "exit"
	while(scanf("%s", cmd) != EOF){
		// a "switch" type structure to call a function to handle each possible command
		if(strcmp(cmd, CMD_INSERT) == 0){
			cmdInsert(fileName);
		}else if(strcmp(cmd, CMD_REMOVE) == 0){
			cmdRemove(fileName);
		}else if(strcmp(cmd, CMD_REQUEST) == 0){
			cmdRequest(fileName);
		}else if(strcmp(cmd, CMD_CREDITS) == 0){
			cmdCredits();
		}else if(strcmp(cmd, CMD_CHANGE_DATAFILE) == 0){
			cmdCF(fileName);
		}else if(strcmp(cmd, CMD_CLEAR) == 0){
			cmdClear();
		}else if(strcmp(cmd, CMD_PWD) == 0){
			cmdPWD();
		}else if(strcmp(cmd, CMD_MKDIR) == 0){
			cmdMkDir();
		}else if(strcmp(cmd, CMD_CD) == 0){
			cmdCd();
		}else if(strcmp(cmd, CMD_LS) == 0){
			cmdLs();
		}else if(strcmp(cmd, CMD_HELP) == 0){
			cmdHelp(argv[PROGNAME]);
		}else if(strcmp(cmd, CMD_EXIT) == 0){
			break; // exit the program
		}else{
			fgets(buf, sizeof buf, stdin); // if the command is invalid, consume the stdin buffer
			buf[strlen(buf)-1] = 0; // as fgets adds the \n to the buffer, we remove it
			printf("Invalid command: %s%s, try typing '%s' for help\n", cmd, buf, CMD_HELP); // says that the command is not valid
		}
		printf("~(%s)~> ", WORKINGDIRECTORY(buf, sizeof buf)); //print the prompt again
	}

	return 0;
	}
