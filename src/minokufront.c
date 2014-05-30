#include "minokubackend.h"

int
main(void)
{
	//ToDo: tidy this, replace cases with constants
	int option;
	tGame game;
	
	randomize();
	option = Menu();
	
	switch (option)
	{
		case 1: /* New Game */
			if (setNewGame(&game)){
			/* DEBUG 
			printf("hidden F C : %d %d\n", game.hiddenboard.rows, game.hiddenboard.columns);
			printf("visual F C : %d %d\n", game.visualboard.rows, game.visualboard.columns);
			printf("gametype: %d\n", game.gametype);
			printf("level: %d\n", game.level);
			printf("moves: %d\n", game.moves);
			printf("undos: %d\n", game.undos);
			printf("mines: %d\n", game.mines);
			printf("mines_left: %d\n", game.mines_left);
			printf("sweeps left: %d\n", game.sweeps_left);	
			*/
			PrintBoard(&game.visualboard);
			PrintBoard(&game.hiddenboard);

			Play(&game);
			}
			else
				printf("No hay suficiente memoria para seguir jugando.");
			
			break;

		case 2:	/* Load */
			break;		
	}

	return 0;
}

int
Menu(void)
{
	int option;
	
	do
	{	
		printf("1. Juego Nuevo\n"  
			   "2. Recuperar Juego Grabado\n"
			   "3. Terminar\n");
	
		option = getint("Elija una opcion: ");

		if (option > 3 || option < 1)
			printf("Ingrese una opcion valida.\n");

	} while(option > 3 || option < 1);
	
	return option;

}

void
setGametypeMenu(tGame * game)
{
	int option;

	do
	{
		printf( "1.1 Juego individual sin limite de movimientos\n"
				"1.2 Juego individual con limite de movimientos\n"
				"1.3 Juego por campaña\n");

		option = getint("Elija una opcion: \n");
		
		if (option > 3 || option < 1)
			printf("Ingrese una opcion valida.\n");
		
	} while(option > 3 || option < 1);

	game->gametype = option;

	return;
}

/* Sets ONLY gameplay config */
int setNewGame(tGame * game)
{
	setGametypeMenu(game);
	getDim(game);
	getLevel(game);
	setGameMinesNumber(game);

 	game->undos = get_undos(game->level);
 	
 	if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
 		game->moves = UNLIMITED_MOVES;
 	else
 		game->moves = get_moves(game->undos, game->mines);
 	
 	game->flags_left = game->mines;

 	if (!CreateHiddenVisualBoard(game))
 		return FALSE;

 	//Ready to play
 	return TRUE;
}

/*
**	CreateHiddenVisualBoard - Creates both hidden
**	and visual board. Returns FALSE when there´s 
** 	no memory left.
*/
int CreateHiddenVisualBoard(tGame * game)
{
	if (!CreateHiddenBoard(&game->hiddenboard, game->mines) || !CreateVisualBoard(&game->visualboard))
 		return FALSE;

 	return TRUE;
}

void getDim(tGame * game)
{
	int rowsaux, colaux;
	
	do
	{
		rowsaux = getint("Ingrese FILAS, minimo 5 y maximo 19:\n");
	} while (rowsaux < 5 || rowsaux > 19);

	do
	{
		colaux = getint("Ingrese COLUMNAS, minimo 5 y maximo 19:\n");
	} while (colaux < 5 || colaux > 19);

	game->visualboard.rows = game->hiddenboard.rows = rowsaux;
	game->visualboard.columns = game->hiddenboard.columns = colaux;

	return;
}

void getLevel(tGame * game){
	
	int level;

	do
	{
		level = getint("Ingrese dificultad:\n1.Facil\n2.Medio\n3.Dificil\n4.Pesadilla\n");
	} while (level < 1 || level > 4);

	game->level = level;

	return;
}

void PrintBoard(tBoard * structboard)
{//ToDo: Tidy this
	int i, j;
	int rows = structboard->rows;
	int columns = structboard->columns;

	char ** board = structboard->board;

	putchar('\t');
	
	for(i = 0; i < columns; i++)
		printf("%d\t", i+1);

	putchar('\n');
	
	for(i = 0; i < rows; i++)
	{
		printf("%c\t", toupperalpha(i));

		for (j = 0; j < columns; j++)
			printf("%c\t", board[i][j]);

		putchar('\n');
	}
}

void Play(tGame * game)
{	
	int legal;
	char won = FALSE; //ToDo: put in tGame
	char end = FALSE;

	tScan scan;
	tCommand command;
	command.command_ref = COMMAND_UNDO;
	command.undo.lastboard.rows = game->visualboard.rows; 
	command.undo.lastboard.columns = game->visualboard.columns;
	CreateBoard(&command.undo.lastboard);
	
	do
	{
		PrintBoard(&game->visualboard); //ToDo: Print all

		do
		{	
			if ((legal = InputCommand(&scan)))
			{	
				if((legal = LegalCommand(&scan, &command)))
					if (scan.scanned_number == 2)
						legal = LegalParams(&game->visualboard, &command, &scan);
				
			}

		if (!legal)
			printf("Commando invalidOO\n");
		} while (!legal);

		//DEBUG
		//printf("Antes de ejecutar (%s): %d\n", command.query.is_row? "Fila":"Columna", command.query.index);
		ExecCommand(game, &command);

	} while(!won && !end);

	return;
}

/*
**		InputCommand - Scans a command and it´s params
**		and saves them in a structure. Returns FALSE if
**		no input. 
*/

int InputCommand(tScan * scan)
{	//ToDo: tidy
	int scanned_number = 1;
	int i, j, k;
	int found_space = FALSE;
	int endfor = FALSE;
	char * rinput; //Result input
	char input[MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2];
	
	printf("Introducir un comando:\n");

	rinput = fgets(input, MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2, stdin);
	puts(input); //DEBUG
	if (rinput == NULL)
		return FALSE;

	//scanned_number = scanf(fmt, scan->command, scan->params);
	DELBFF();

	// Exit if no scan
	if (input[0] == '\n')
		return FALSE;
	printf("Leggar a voooos\n");
	for (i = 0, j = 0, k = 0; input[i] && !endfor; i++)
	{
		if (input[i] == ' ')
		{	
			if (input[i+1] != '\n')
				scanned_number++;
			
			if (scanned_number > 2)
				endfor = TRUE;
			
			found_space = TRUE;
		}
		else if (!found_space)
			scan->command[j++] = input[i];
		else
			scan->params[k++] = input[i];
	}

	scan->command[j] = '\0';
	scan->params[k] = '\0';
	scan->scanned_number = scanned_number;
	puts(scan->command); puts(scan->params);

	return TRUE;
}

/*
**		LegalCommand - Receives scanned command, sets 
**		command reference in structure,retuns TRUE 
**		if valid.
*/

int LegalCommand(tScan * scan, tCommand * command)
{
	//ToDo: Reduce mem access in for
	/* Hardcoded commands respecting the COMMAND_ defines order */
	static char * commandlist[] = {"s", "flag", "unflag", "query", "save", "quit", "undo"};
	
	char found = FALSE;
	int commandindex;

	for (commandindex = 0; commandindex < COMMANDS_NUMBER && !found; commandindex++)
	{
		if (!strcmp(scan->command, commandlist[commandindex]))
		{
			/* Assigns the command reference respecting the 
			   COMMAND_ defines order */
			command->command_ref = commandindex;
			found = TRUE;
			//DEBUG
			printf("DEBUG: LEGALCOMMAND FOUND: REF: %d\n",command->command_ref);
		}
	}

	if (!found)
		return FALSE;

	return TRUE;
}

int
LegalParams(tBoard * visualboard, tCommand * structcommand, tScan * scan)
{	
	switch(structcommand->command_ref)
	{
		case COMMAND_SWEEP:
			//printf("EN el switch: %s\n", scan->params);
			return LegalSweep(visualboard, structcommand, scan->params);
		
		case COMMAND_FLAG:
			return LegalFlag(visualboard, structcommand, scan->params, DO_FLAG);

		case COMMAND_UNFLAG:
			return LegalFlag(visualboard, structcommand, scan->params, DO_UNFLAG);

		case COMMAND_QUERY:	
			return LegalQuery(visualboard, structcommand, scan->params);
		
		case COMMAND_SAVE:
			//return LegalSave(scan);
		return TRUE;
	
	}
	return TRUE;
}

int
LegalSweep(tBoard * visualboard, tCommand * structcommand, char * params)
{//todo: tidy
	tPos aux;
	char * closepos;
	char legal = TRUE;
	char i_scan;
	int i;
	//DEBUG
	//printf("ENTRE\n");
	//printf("%s\n", params);

	if (sscanf(params, "(%c,%d", &i_scan, &aux.j) != 2)
		return FALSE;

	if ((closepos = strchr(params,')')) == NULL)
		return FALSE;
	else
		if (*(closepos +1) != '\n')
			return FALSE;

	//printf ("%d\n", *(strchr(params, ')')+1));
	//ToDo: Modularize
	i_scan = get_row_pos_byref(i_scan);
	aux.i = (int)i_scan;
	aux.j--;
	//DEBUG
	//printf("AFTER SCAN %d %d\n", aux.i, aux.j);
	//printf("%s\n", LegalPos(visualboard, &aux)?"Si":"No" );
	if (!isupper('A' + aux.i)) // If Column is not a letter return false
		legal = FALSE;

	else if (!LegalPos(visualboard, &aux)) // If Position is not on the board return false
		legal = FALSE;
	
	else if (visualboard->board[aux.i][aux.j] != VISUAL_UNFLAGGED)  // If there's a '&' or '-' on the visual board return false
		legal = FALSE;

	if (legal){
		structcommand->sweep.i = aux.i;
		structcommand->sweep.j = aux.j;
		printf("AFTER SAVE STRUCT %d %d\n", structcommand->sweep.i, structcommand->sweep.j);
	}

	return legal;

}			

int

LegalFlag(tBoard * visualboard, tCommand * structcommand, char * params, char task) /*No valida si ya esta flaggeado*/
{	//Tidy
	//int fposi, fposj, lposi, lposj;
	int i;
	char legal = TRUE;
	char legal_range = FALSE;
	tPos f_aux;
	tPos l_aux;
	char f_scan;
	char l_scan;
	

	char * closepos;

		if ((closepos = strchr(params,')')) == NULL)
			return FALSE;
		else
			if (*(closepos +1) != '\n')
				return FALSE;

	// Checks if range is legal
	if (sscanf(params, "(%c,%d:%c,%d", &f_scan, &f_aux.j, &l_scan, &l_aux.j) == 4)
	{
		f_scan = get_row_pos_byref(f_scan);
		f_aux.i = f_scan;
		l_scan = get_row_pos_byref(l_scan);
		l_aux.i = l_scan;
		f_aux.j--;
		l_aux.j--;

		// Syntax check
		if (!isupper('A' + f_aux.i)|| !isupper('A' + l_aux.i))
			legal = FALSE;
		
		else if (!LegalPos(visualboard, &f_aux) || !LegalPos(visualboard, &l_aux))
			legal = FALSE;

		// Legal move check
		else if (f_aux.i == l_aux.i)
		{	
			if(f_aux.j > l_aux.j)
				legal = FALSE;
			else
				structcommand->flag.is_row = TRUE;
		}	
		else if (f_aux.j == l_aux.j)
		{	
			if(f_aux.i > f_aux.i)
				legal = FALSE;
			else
				structcommand->flag.is_row = FALSE;
		}	
		else
			legal = FALSE;
		
		if(structcommand->flag.is_row)
		{
			for(i=f_aux.j; i<l_aux.j; i++)
			{	
				if ((task == DO_FLAG) && (visualboard->board[f_aux.i][i] == VISUAL_FLAGGED))
					legal_range = TRUE;
				else if( (task == DO_UNFLAG) && (visualboard->board[f_aux.i][i] == VISUAL_FLAGGED))
					legal_range = TRUE;
			}	
		}
		else
		{
			for(i=f_aux.i; i<l_aux.i; i++)
			{	
				if ((task == DO_FLAG) && (visualboard->board[i][f_aux.j] == VISUAL_FLAGGED))
					legal_range = TRUE;
				else if( (task == DO_UNFLAG) && (visualboard->board[i][f_aux.j] == VISUAL_FLAGGED))
					legal_range = TRUE;
			}		
		}	
		
		if (legal && legal_range)
		{
			structcommand->flag.is_range = TRUE;
			structcommand->flag.first_pos.i = f_aux.i;
			structcommand->flag.first_pos.j = f_aux.j;
			structcommand->flag.last_pos.i 	= l_aux.i;
			structcommand->flag.last_pos.j 	= l_aux.i;
		}

	}
	else if (sscanf(params, "(%c,%d)", &f_scan, &f_aux.j) == 2)
	{
		f_scan = get_row_pos_byref(f_scan);
		f_aux.i = f_scan;
		f_aux.j--;
		
		if (!isupper('A' + f_aux.i))
			legal = FALSE;

		else if(!LegalPos(visualboard, &f_aux))
			legal = FALSE;
		
		else if ( (task == DO_FLAG) && (visualboard->board[f_aux.i][f_aux.j] != VISUAL_UNFLAGGED))
			legal = FALSE;
		else if( (task == DO_UNFLAG) && (visualboard->board[f_aux.i][f_aux.j] != VISUAL_FLAGGED))
			legal = FALSE;
		
		if (legal)
		{	
			structcommand->flag.is_range 	= FALSE;
			structcommand->flag.first_pos.i = f_aux.i;
			structcommand->flag.first_pos.j = f_aux.j;
		}			
	} 	
	return legal;
}

int
LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params)
{	//ToDo: tidy
	char index_row;
	int index_column;
	char legal = TRUE;
	
	if(sscanf(params, "%d", &index_column) == 1)
	{	
		index_column--;
		if (index_column < 0 || index_column > visualboard->columns)
			legal = FALSE;
		if (legal)
		{
			structcommand->query.is_row = FALSE;
			structcommand->query.index = index_column;
		}	
	}
	else if (sscanf(params, "%c", &index_row) == 1 )
	{	
		index_row = get_row_pos_byref(index_row);
		if (!isupper('A' + index_row))
			legal = FALSE;
		else if (index_row < 0 || index_row > visualboard->rows)
			legal = FALSE;
		if(legal)
		{	
			structcommand->query.is_row = TRUE;
			structcommand->query.index = index_row;
		}
	}
	
	return legal;	

}

void PrintQuery (tQuery * query)
{	
	int i;
	int dim = query->results.dim;

	for (i = 0; i < dim; i++)
		printf("%d%s", query->results.results[i], (i != (dim-1))? " - ": "");	
	putchar('\n');

	return;
}
