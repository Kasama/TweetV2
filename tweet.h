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

#ifndef TWEET_H
#define TWEET_H 

// ascii character "Unit Separator"
#define   END_FIELD   31

/*
 * Internal Structure of a Tweet:
 *
 * +--------+----------------+
 * |  Type  |   Field Name   |
 * +--------+----------------+
 * | char*  | text		     |
 * | char*  | userName		 |
 * | char*  | coords		 |
 * | char*  | language		 |
 * | int    | favoriteCount	 |
 * | int    | retweetCount	 |
 * | long   | viewsCount	 |
 * +--------+----------------+
 * 
 */
typedef struct tweet TWEET;

/*
 * Constructs a tweet
 *
 * An END_FIELD character will be added 
 * to the end of each string character
 *
 */
// Kike
TWEET *newTweet(				\
		char   *text,           \
		char   *userName,       \
		char   *coords,         \
		char   *language,       \
		int    favoriteCount,   \
		int    retweetCount,    \
		long   viewsCount       \
);

/*
 * Writes the tweet into the file
 * 
 * byteOffset = bestFit(tweet, *regSize)
 * fwrite(sizeof(tweet), byteOffset);
 * fwrite(tweet, byteOffset+sizeof(int))
 * if ((regSize-sizeof(tweet)) >= sizeof(int)){
 *   fwrite(-(regSize-sizeof(tweet)), byteOffset+sizeof(int)+sizeof(tweet))
 * }
 *
 */
// Fred
int writeTweet(char *filename, TWEET *tweet);

/*
 * Reads a tweet from byteoffset
 *
 * regSize = fread(offset)
 * tweet = fread(offset+sizeof(int))
 */
// Kike
TWEET *readTweet(char *filename, long offset);

/*
 * sequential search by user in the register file
 */
long *findAllTweets(char *filename, long *foundOccurences);

/*
 * sequential search by user in the register file
 */
// Carlos
long *findOffsetByUser(char *filename, char *username, long *foundOccurences);

/*
 * binary seach by favorite count in the index file
 */
// Justus
long *findDataOffsetByFavoriteCount(char *filename, int favoriteCount, long *foundOccurences);

/*
 * binary seach by language in the index file
 */
// Justus
long *findDataOffsetByLanguage(char *filename, char* language, long *foundOccurences);

/*
 * logically remove the tweet in the data file by its offset
 * logically remove the tweet in the index file by its offset
 */
// Crocomo
int removeTweet(char *filename, long offset);

// Crocomo
void printTweet(TWEET *tweet);

void destroyTweet (TWEET **tweet);

#endif
