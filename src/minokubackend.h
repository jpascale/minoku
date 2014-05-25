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
#define NOT_FOUND_MINE 	0
#define FOUND_MINE 		1

// Sweep 
#define SWEEP_MINE -1

// Chars for hidden board
#define HIDDEN_MINE 	'#'
#define HIDDEN_EMPTY 	'-'

// Chars for visual display
#define VISUAL_UNFLAGGED 	'0'
#define VISUAL_FLAGGED 		'&'
#define VISUAL_EMPTY 		'-'

#define GAMETYPE_INDIVIDUAL_NOLIMIT 0
#define GAMETYPE_INDIVIDUAL_LIMIT 	1
#define GAMETYPE_CAMPAIGN 			2

#define UNLIMITED_MOVES 0

// Level mines percentage
#define PERCENT_EASY 		0.2
#define PERCENT_MEDIUM 		0.5
#define PERCENT_HARD 		0.7
#define PERCENT_NIGHTMARE 	0.9

// Command constants
#define MAX_COMMAND_LEN 	8
#define MAX_PARAMS_LEN 		24
#define COMMANDS_NUMBER 	7
#define MAX_FILENAME_LEN 	24

#define COMMAND_SWEEP	0
#define COMMAND_FLAG	1
#define COMMAND_UNFLAG 	2
#define COMMAND_QUERY 	3
#define COMMAND_SAVE	4
#define COMMAND_QUIT 	5
#define COMMAND_UNDO 	6

// Flag/Unflag tasks
#define DO_FLAG		0
#define DO_UNFLAG 	1

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

// Delete buffer
#define DELBFF() while(getchar() != '\n')

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
	int mines_left; //number of mines not flagged
	int sweeps_left; //number of positions without sweep

} tGame;

typedef struct
{
	int i;
	int j;

} tPos;

typedef struct
{
	tPos lastpos;
	int last_play;
	char last_was_undo;

} tUndo;

typedef struct
{
	int * results; //ToDo: change for array
	int dim;

} tArray;

typedef struct
{
	int scanned_number;
	char command[MAX_COMMAND_LEN];
	char params[MAX_PARAMS_LEN];

} tScan;

typedef struct 
{
	tPos first_pos;
	tPos last_pos;
	char is_range;
	char is_row;

} tFlag;

typedef struct 
{
	char index;
	char is_row;

} tQuery;

typedef struct
{
	int command_ref;
	tPos sweep;
	tFlag flag;
	tQuery query;
	char save_filename[MAX_FILENAME_LEN];

} tCommand;

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
int setNewGame(tGame * game);
void Play(tGame * game);
int LegalCommand(tScan * scan, tCommand * command);
int InputCommand(tScan * scan);

/*
**		Function prototypes (back)
*/
void setGameMinesNumber(tGame * game);
int CreateBoard(tBoard * structboard);
int InitBoardMines(tBoard * structboard, int mines);
void InitBoard(tBoard * structboard, char initchar);
int CreateVisualBoard(tBoard * structboard);
int CreateHiddenBoard(tBoard * structboard, int mines);
int Query(tBoard * structboard, tArray * pquery, int element, char isrow, int block);
int DoFlagUnflag(tGame * game, tPos * pos, char task);
int Sweep(tGame * game, tPos * position);
int LegalPos(tBoard * structboard, tPos * position);
