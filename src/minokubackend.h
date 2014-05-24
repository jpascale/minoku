//_____minokubackend.h_____//

/*
**		Includes
*/
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getnum.h"
#include "random.h"

/*
**		Macros
*/
#define DEBUG

#define FALSE 0
#define TRUE 1

// Printf color constants - Usage: printf("%sasdasd\n", KRED);
#define KNRM  "\x1B[0m" //default
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// Levels
#define EASY 		1
#define MEDIUM		2
#define HARD 		3
#define NIGHTMARE	4

// Query States
#define NOT_FOUND_MINE 0
#define FOUND_MINE 1

// Chars for hidden board
#define HIDDEN_MINE 	'#'
#define HIDDEN_EMPTY 	'-'

// Chars for visual display
#define VISUAL_UNFLAGGED 	'0'
#define VISUAL_FLAGGED 		'&'
#define VISUAL_EMPTY 		'-'

#define GAMETYPE_INDIVIDUAL_NOLIMIT 0
#define GAMETYPE_INDIVIDUAL_LIMIT 1
#define GAMETYPE_CAMPAIGN 2

#define UNLIMITED_MOVES 0

// Level mines percentage
#define PERCENT_EASY 0.2
#define PERCENT_MEDIUM 0.5
#define PERCENT_HARD 0.7
#define PERCENT_NIGHTMARE 0.9

// map undos quantity
#define get_undos(level) (((level)==NIGHTMARE)?1: \
						  ((level)==HARD)?3: \
						  ((level)==MEDIUM)?5:10)

// Gets player moves
#define get_moves(mines, undos) ((mines) + (undos))

// Maps letter reference to board row number
#define get_row_pos_byref(row) (toupper(row) - 'A')

// Number to upper letter
#define toupperalpha(x) ((x)+'A')

// Maximum and minimum
#define min(x, y) ((x)<(y)?(x):(y))
#define max(x, y) ((x)>(y)?(x):(y))

/*
**		Structs
*/
typedef struct
{
	char ** board;
	int rows;
	int columns;

} tBoard;

typedef struct 
{
	tBoard visualboard;
	tBoard hiddenboard;
	int gametype;
	int level;
	int moves;
	int undos;
	int mines;

} tGame;

typedef struct
{
	int x;
	int y;

} tPos;

typedef struct
{
	tPos lastpos;
	int last_play;
	char last_was_undo;

} tUndo;

typedef struct
{
	int * results;
	int dim;

} tQuery;

/*
**		Function prototypes (front) 
**		TODO: Put this in frontend. Backend does not need this.
**		>>Stays here for now only for clarity.
*/
int Menu(void);
void setGametypeMenu(tGame * game);
void PrintBoard(tBoard * structboard);
void getLevel(tGame * game);
void getDim(tGame * game);
void setNewGame(tGame * game);
void Play(tGame * game);
void InputCommand(tGame * game);

/*
**		Function prototypes (back)
*/
void setGameMinesNumber(tGame * game);
int CreateBoard(tBoard * structboard);
int InitBoardMines(tBoard * structboard, int mines);
void InitBoard(tBoard * structboard, char initchar);
int CreateVisualBoard(tBoard * structboard);
int CreateHiddenBoard(tBoard * structboard, int mines);
int Query(tBoard * structboard, tQuery * pquery, int element, char isrow, int block);
