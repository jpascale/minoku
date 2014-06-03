#include "minokubackend.h"


/*
**		Function prototypes (front) 
*/
int Menu(void);
void setGametypeMenu(tGame * game);
void PrintBoard(tBoard * structboard);
void getLevel(tGame * game);
/*
**	getDim - Asks for board dim.
*/
void getDim(tGame * game);
/*
**	setNewGame - Sets All the necesary info to play 
**	in game structure.  
*/ 
int setNewGame(tGame * game);
void Play(tGame * game);
int LegalCommand(tScan * scan, tCommand * command);
int InputCommand(tScan * scan);
int LegalParams(tGame * game, tCommand * command, tScan * scan);
int LegalSweep(tBoard * visualboard, tCommand * command, char * params);
int LegalFlag(tGame * game, tCommand * command, char * params, char task);
int LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params);
void PrintQuery (tQuery * query);
int AskUndo(tGame * game, tUndo * undo);
int ExecCommand(tGame *game, tCommand *command);
void getName(char * name);
void PrintAll(tGame * game, tCommand * command);
/*
**	getCampaignName - Gets name and checks if it ends with ".txt"
*/
void getCampaignName(tGame *game);
int setCampaign(tGame * game); 
int resumeCampaign(tGame * game);
int CheckLegalPos(tBoard * structboard, tPos * pos);
int LegalSave(tCommand * structcommand, char * params);
void TranslateCoords(tPos * pos);
void Quit(tGame * game, tCommand * command);
void PrintResult(tGame * game);

int
main(void)
{
	int option;
	tGame game;
	char loadname[MAX_FILENAME_LEN];
	int valid;
	
	randomize();

	option = Menu();
	
	
	switch (option)
	{
		case 1: /* New Game */
			
			setGametypeMenu(&game);
			if (game.gametype != GAMETYPE_CAMPAIGN)
			{
				game.campaign_level = 0;
				if (setNewGame(&game))
					Play(&game);
				else
					printf("%sNo hay suficiente memoria para seguir jugando.\n%s", KERR, KDEF);
			}
			else
			{
				game.campaign_level = 1;
				do
				{	
					if (!(valid = setCampaign(&game)))
						printf("%sArchivo invalido o inexistente\n%s", KERR, KDEF);
				}
				while (!valid);
			}
			break;

		case 2:	/* Load */
			do
			{
				getName(loadname);
				if (!(valid = LoadFile(&game, loadname)))
					printf("%sArchivo invalido o inexistente%s\n", KERR, KDEF);
				else
					if (game.gametype == GAMETYPE_CAMPAIGN)
					{
						if (!(valid = resumeCampaign(&game)))
							printf("%sArchivo de campaña invalido o inexistente.%s\n", KERR, KDEF);
					}

			} while (!valid);
			
			Play(&game);
			break;		
	}

	return 0;
}

int setCampaign(tGame * game)
{
	int i, valid;
	getCampaignName(game);
	

	if (!LoadCampaign(game))
		return FALSE;

	for (i = 0; i < game->levels_amount; i++)
	{
		if ((valid = setNewGame(game)) == MALLOC_ERR)
		{
			printf("%sNo hay memoria suficiente para seguir jugando.%s\n", KASK, KDEF);
			return FALSE;
		}
		else if (valid)
			Play(game);
	}
	return TRUE;
}

int resumeCampaign(tGame * game)
{
	int i, valid;
	int campaign_rows;
	int campaign_columns;

	if (!LoadCampaign(game))
		return FALSE;
	campaign_rows = game->campaign[game->campaign_level-1].rows;
	campaign_columns = game->campaign[game->campaign_level-1].columns;
	
	if (campaign_rows != game->hiddenboard.rows || campaign_columns != game->hiddenboard.columns)
		return FALSE;

	Play(game);
	
	for (i = game->campaign_level; i < game->levels_amount; i++)
	{
		if ((valid = setNewGame(game)) == MALLOC_ERR)
		{
			printf("%sNo hay suficiente memoria para seguir jugando.\n%s", KERR, KDEF);
			return FALSE;
		}
		else if (valid)				
			Play(game);
	}
	return TRUE;
}
int Menu(void)
{
	int option;
	
	do
	{	
		printf("%s1. Juego Nuevo\n"  
			   "2. Recuperar Juego Grabado\n"
			   "3. Terminar%s\n", KASK, KDEF);
	
		option = getint("%sElija una opcion: %s", KASK, KDEF);

		if (option > 3 || option < 1)
			printf("%sIngrese una opcion valida.%s\n", KERR, KASK);

	} while(option > 3 || option < 1);
	
	return option;

}

void setGametypeMenu(tGame * game)
{
	int option;

	do
	{
		printf( "%s1.1 Juego individual sin limite de movimientos\n"
				"1.2 Juego individual con limite de movimientos\n"
				"1.3 Juego por campaña%s\n", KASK, KDEF);

		option = getint("%sElija una opcion: %s\n", KASK, KDEF);
		
		if (option > 3 || option < 1)
			printf("%sIngrese una opcion valida.%s\n", KERR, KDEF);
		
	} while(option > 3 || option < 1);

	game->gametype = option-1;

	return;
}


int setNewGame(tGame * game)
{

	if (game->gametype != GAMETYPE_CAMPAIGN)
	{
		getDim(game);
		getLevel(game);
	}
	else
	{
		int campaign_rows = game->campaign[game->campaign_level-1].rows;
		int campaign_columns = game->campaign[game->campaign_level-1].columns;
		int campaign_level = game->campaign[game->campaign_level-1].level;

		if (campaign_rows > MAX_ROWS || campaign_columns > MAX_COLUMNS)
		{	
			printf("El nivel actual posee un tablero muy grande, se pasara al proximo nivel");
			game->campaign_level++;
			return FALSE;
		}
		
		game->visualboard.rows = game->hiddenboard.rows = campaign_rows;
		game->visualboard.columns = game->hiddenboard.columns = campaign_columns;
		game->level = campaign_level;
	}
	setGameMinesNumber(game);
 	game->undos = get_undos(game->level);
 	
 	//Moves
 	if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
 		game->moves = UNLIMITED_MOVES;
 	else
 		game->moves = get_moves(game->undos, game->mines);
 	
 	game->mines_left = game->mines;
 	game->sweeps_left = (game->visualboard.rows * game->visualboard.columns) - game->mines;
 	game->flags_left = game->mines;
 	game->gamestate = GAMESTATE_DEFAULT; 

 	if (!CreateHiddenVisualBoard(game))
 		return MALLOC_ERR;

 	//Ready to play
 	return TRUE;
}


void getCampaignName(tGame *game)
{
	char name[MAX_FILENAME_LEN];
	int valid;
	int len;

	do
	{
		printf("%sEscriba nombre de campaña, el archivo debe terminar en .txt%s\n", KASK, KDEF);
		
		if (gets(name) == NULL)
			valid = FALSE;
		else
		{	
			len = strlen(name);
			if (len < FORMAT_LENGTH + 1)
				valid = FALSE;
			else
			{
				if (strstr(&(name[len-FORMAT_LENGTH]), FILE_FORMAT) == NULL)
					valid = FALSE;
				else
					valid = TRUE;
			}
		}
	} while (!valid);
	
	strcpy(game->campaign_name, name);	
	return;
}


void getDim(tGame * game)
{
	int rowsaux, colaux;
	
	do
	{
		rowsaux = getint("%sIngrese FILAS, minimo 5 y maximo 19:%s\n", KASK, KDEF);
	} while (rowsaux < 5 || rowsaux > 19);

	do
	{
		colaux = getint("%sIngrese COLUMNAS, minimo 5 y maximo 19:%s\n", KASK, KDEF);
	} while (colaux < 5 || colaux > 19);

	game->visualboard.rows = game->hiddenboard.rows = rowsaux;
	game->visualboard.columns = game->hiddenboard.columns = colaux;

	return;
}

/*
**	getLevel - Asks for level taking into account nightmare restriction.
*/
void getLevel(tGame * game)
{	
	int level;
	int can_nightmare = ((game->visualboard.rows * game->visualboard.columns) >= 100);
	int can = FALSE;

	do
	{
		level = getint(	"%sIngrese dificultad:\n"
						"%s1.Facil\n"
						"%s2.Medio\n"
						"%s3.Dificil\n"
						"%s4.Pesadilla%s\n", KREF, KSWP, KFLG, KREF, KMIN, KDEF);
	
		if (level == NIGHTMARE)
			can_nightmare? (can = TRUE) : (can = FALSE);
		else
			can = TRUE;

		if (!can)
			printf("%sNo es posible elegir pesadilla con menos de 100 casilleros.%s\n", KERR, KDEF);

	} while(level < EASY || level > NIGHTMARE || !can);

	game->level = level;

	return;
}

/*
**	PrintBoard
*/
void PrintBoard(tBoard * structboard)
{
	int i, j;
	int rows = structboard->rows;
	int columns = structboard->columns;

	char ** board = structboard->board;

	putchar('\t');
	
	printf("%s", KREF);	
	for(i = 0; i < columns; i++)
		printf("%d\t", i+1);

	printf("%s\n", KDEF);
	
	for(i = 0; i < rows; i++)
	{
		printf("%s%c%s\t", KSWP, toupperalpha(i), KDEF);

		for (j = 0; j < columns; j++)
			printf("%s%c%s\t", COLORBOARD, board[i][j], KDEF);

		putchar('\n');
	}
}

/*
**	Play - Receives gameplay configured structure and starts game.
*/
void Play(tGame * game)
{	
	int legal;

	tScan scan;
	tCommand command;
	command.undo.can_undo = FALSE;
	command.undo.lastboard.rows = game->visualboard.rows; 
	command.undo.lastboard.columns = game->visualboard.columns;
	CreateBoard(&command.undo.lastboard);;

	do
	{
		PrintAll(game, &command);
		
		do
		{	
			if ((legal = InputCommand(&scan)))
			{	
				if((legal = LegalCommand(&scan, &command)))
					if (command.command_ref < 5) //All commands but quit or undo
						legal = LegalParams(game, &command, &scan);
			}

		if (!legal)
			printf("%sComando invalido.%s\n", KERR, KDEF);

		} while (!legal);

		ExecCommand(game, &command);
		CheckGameState(game);
	} while(game->gamestate == GAMESTATE_DEFAULT);

	PrintResult(game);

	freeBoard(game->hiddenboard.board, game->hiddenboard.rows);
	freeBoard(game->visualboard.board, game->hiddenboard.rows);
	return;
}

/*
**		InputCommand - Scans a command and it´s params
**		and saves them in a structure. Returns FALSE if
**		no input. 
*/

int InputCommand(tScan * scan)
{	//ToDo: tidy
	int i, j, k;
	int found_space = FALSE;
	int endfor = FALSE;
	char * rinput; //Result input
	char input[MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2]; //Command, params and 2 for space and '\n'
	
	printf("%sIntroducir un comando: %s", KASK, KDEF);

	rinput = fgets(input, MAX_COMMAND_LEN + MAX_PARAMS_LEN + 2, stdin); //+2 0 and blank
	
	if (rinput == NULL)
		return FALSE;

	// Exit if no scan
	if (input[0] == '\n')
		return FALSE;

	for (i = 0, j = 0, k = 0; input[i] && !endfor; i++)
	{
		if (input[i] == ' ')
		{	
			if (!found_space && input[i+1] != '\n')
				found_space = TRUE;
			else
				endfor = TRUE;
		}

		else if (!found_space)
			scan->command[j++] = input[i];
		else
			scan->params[k++] = input[i];
	}

	scan->command[j] = '\0';
	scan->params[k]	 = '\0';

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
	static char * commandlist[] = {"s", "flag", "unflag", "query", "save", "quit\n", "undo\n"};
	
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
		}
	}

	return found;
}

/*
**	LegalParams - Detects command reference and returns command validation.
*/
int LegalParams(tGame * game, tCommand * structcommand, tScan * scan)
{	
	switch(structcommand->command_ref)
	{
		case COMMAND_SWEEP:
			return LegalSweep(&game->visualboard, structcommand, scan->params);
		
		case COMMAND_FLAG:
			return LegalFlag(game, structcommand, scan->params, DO_FLAG);

		case COMMAND_UNFLAG:
			return LegalFlag(game, structcommand, scan->params, DO_UNFLAG);

		case COMMAND_QUERY:	
			return LegalQuery(&game->visualboard, structcommand, scan->params);
		
		case COMMAND_SAVE:
			return LegalSave(structcommand, scan->params);
	}
	return TRUE;
}

int LegalSave(tCommand * structcommand, char * params)
{
	params[strlen(params)-1] = '\0';
	
	if (! *params)
		return FALSE;
	strcpy(structcommand->save_filename, params);
	return TRUE;
}

int LegalSweep(tBoard * visualboard, tCommand * structcommand, char * params)
{
	tPos aux;
	char new_line;
	char legal = TRUE;
	char i_scan;

	if (sscanf(params, "(%c,%d)%c", &i_scan, &aux.j, &new_line) != 3)
		return FALSE;

	if (new_line != '\n')
		return FALSE;
	/*	Used &i_scan because we cannot use int pointer,
	**	we use char pointer and cast it instead
	*/
	aux.i = (int)i_scan;

	if (!CheckLegalPos(visualboard, &aux))
		legal = FALSE;
	/* If there's an '&' or '-' on the visual board return false*/
	else if (visualboard->board[aux.i][aux.j] != VISUAL_UNFLAGGED)  
		legal = FALSE;

	if (legal){
		structcommand->sweep.i = aux.i;
		structcommand->sweep.j = aux.j;
	}

	return legal;

}

int
LegalFlag(tGame * game, tCommand * structcommand, char * params, char task) /*No valida si ya esta flaggeado*/
{	//ToDo: Tidy
	int i;
	char legal = TRUE;
	char range_count = 0;
	tPos f_aux;
	tPos l_aux;
	char f_scan;
	char l_scan;
	char new_line;

	/* Range flag*/
	if (sscanf(params, "(%c,%d:%c,%d)%c", &f_scan, &f_aux.j, &l_scan, &l_aux.j, &new_line) == 5)
	{	
		if (new_line != '\n')
			return FALSE;

		f_aux.i = (int)f_scan;
		l_aux.i = (int)l_scan;
		
		if (!CheckLegalPos(&game->visualboard, &f_aux) || !CheckLegalPos(&game->visualboard, &l_aux))
			legal = FALSE;
		/*Check if it's a row or a column and that the first position comes
		before the last position*/
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
		/*Check how many elements you are flagging*/
		if(legal && structcommand->flag.is_row)
		{
			for(i=f_aux.j; i<=l_aux.j; i++)
			{	
				if ((task == DO_FLAG) && (game->visualboard.board[f_aux.i][i] == VISUAL_UNFLAGGED))
					range_count++;
				else if( (task == DO_UNFLAG) && (game->visualboard.board[f_aux.i][i] == VISUAL_FLAGGED))
					range_count++;
			}	
		}
		else if (legal)
		{
			for(i=f_aux.i; i<=l_aux.i; i++)
			{	
				if ((task == DO_FLAG) && (game->visualboard.board[i][f_aux.j] == VISUAL_UNFLAGGED))
					range_count++;
				else if( (task == DO_UNFLAG) && (game->visualboard.board[i][f_aux.j] == VISUAL_FLAGGED))
					range_count++;
			}		
		}	
		/*Check if there's nothing to flag in the range
		or if you don't have enough moves or flags left*/
		if (legal)
		{
			if (range_count == 0)
				legal = FALSE;
			else if ( (task == DO_FLAG) && (range_count>game->flags_left) )
				legal = FALSE;
			else if( (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT && range_count>game->moves))
				legal = FALSE;	
		}
		if (legal)
		{	
			structcommand->flag.is_range = TRUE;
			structcommand->flag.first_pos.i = f_aux.i;
			structcommand->flag.first_pos.j = f_aux.j;
			structcommand->flag.last_pos.i 	= l_aux.i;
			structcommand->flag.last_pos.j 	= l_aux.j;
		}

	}
	/*Single Flag*/
	else if (sscanf(params, "(%c,%d)%c", &f_scan, &f_aux.j, &new_line) == 3)
	{
		if (new_line != '\n')
			return FALSE;	
		
		f_aux.i = (int)f_scan;

		if (!CheckLegalPos(&game->visualboard, &f_aux))
			legal = FALSE;

		/*Check the element you are flagging is valid and you have enough flags left*/
		else if ( (task == DO_FLAG) && (game->visualboard.board[f_aux.i][f_aux.j] != VISUAL_UNFLAGGED))
			legal = FALSE;
		else if( (task == DO_UNFLAG) && (game->visualboard.board[f_aux.i][f_aux.j] != VISUAL_FLAGGED))
			legal = FALSE;
		else if( task == DO_FLAG && game->flags_left == 0)
			legal = FALSE;
		
		if (legal)
		{	
			structcommand->flag.is_range 	= FALSE;
			structcommand->flag.first_pos.i = f_aux.i;
			structcommand->flag.first_pos.j = f_aux.j;
		}			
	}
	else
		legal = FALSE; 	
	
	return legal;
}

int
LegalQuery(tBoard * visualboard, tCommand * structcommand, char * params)
{
	char index_row;
	int index_column;
	char new_line;
	char legal = TRUE;

	/*Query Column*/
	if(sscanf(params, "%d%c", &index_column, &new_line) == 2)
	{	
		/*Translate the column to backend*/
		index_column--;

		if (new_line != '\n')
			return FALSE;
		/*Check the column is inside the board*/
		if (index_column < 0 || index_column >= visualboard->columns)
			legal = FALSE;
		if (legal)
		{
			structcommand->query.is_row = FALSE;
			structcommand->query.index = index_column;
		}	
	}
	/*Query Row*/
	else if (sscanf(params, "%c%c", &index_row, &new_line) == 2 )
	{	
		/*Translate the row to backend*/
		index_row = get_row_pos_byref(index_row);
		
		if (new_line != '\n')
			return FALSE;
		
		/*Check the row was a letter*/
		if (!isupper('A' + index_row))
			legal = FALSE;
		
		/*Check the row is inside the board*/
		else if (index_row < 0 || index_row >= visualboard->rows)
			legal = FALSE;
		
		if(legal)
		{	
			structcommand->query.is_row = TRUE;
			structcommand->query.index = index_row;
		}
	}
	else
		legal = FALSE;
	
	return legal;

}

void PrintQuery(tQuery * query)
{	
	int i;
	int dim = query->results.dim;

	printf("Query: ");

	if (dim)
	{
		for (i = 0; i < dim; i++)
			printf("%d%s", query->results.results[i], (i != (dim-1))? " - ": "\n");	
	}else
		printf("0\n");
		
	return;
}

int AskUndo(tGame * game, tUndo * undo)
{
	char input[MAX_COMMAND_LEN];
	char * pinput;

	int wasundo = FALSE;
	int wasquit = FALSE;

	PrintBoard(&game->visualboard);

	printf("%sPerdiste! ¿Hacer Undo? (Ingresar undo o quit)%s\n", KASK, KDEF);

	do{
		pinput = fgets(input, MAX_COMMAND_LEN, stdin);
		if (pinput != NULL)
		{
			wasundo = (strcmp(input,"undo\n") == 0);
			wasquit = (strcmp(input,"quit\n") == 0);
		}

		if (!wasundo && !wasquit)
			printf("%sIngresar quit o undo.%s\n", KERR, KDEF);
	}
	while ( (!wasundo && !wasquit) || (pinput == NULL));


	if (wasundo)
	{
		Undo(game, undo);
		game->undos--;
		if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
			game->moves--;
		return TRUE;
	}
	else
	{
		game->gamestate = GAMESTATE_LOSE;
		return FALSE;
	}

}

void PrintAll(tGame * game, tCommand * command)
{
	DELSHELL();

	PrintBoard(&game->visualboard);
	
	printf("%s", KMSG);
	if (game->gametype)
		printf("Movimientos restantes: %d\n", game->moves);
	
	printf("Undos restantes: %d\n", game->undos);
	printf("Flags restantes: %d\n", game->flags_left);
	if (command->command_ref == COMMAND_QUERY)
	{
		PrintQuery(&command->query);
		free(command->query.results.results);
	}
	printf("%s", KDEF);
	return;
}

void Quit(tGame * game, tCommand * command)
{
	char * pinput;
	char input[5];
	char savename[MAX_FILENAME_LEN];
	int yes = FALSE;
	int no = FALSE;

	printf("%sDesea guardar la partida? (Ingrese si o no)%s\n", KASK, KDEF);
	do
	{
		pinput = fgets(input, 5, stdin);
		if (pinput != NULL)
		{
			yes = strcmp(input, "si\n") == 0;
			no = strcmp(input, "no\n") == 0;
		}
		if (!yes && !no)
			printf("%sIngresar si o no%s\n", KASK, KDEF);

	}while((!yes && !no) || (pinput == NULL));

	if (yes)
	{
		getName(savename);
		WriteSaveFile(game, savename);
	}
	else
		exit(0);	
}

int ExecCommand(tGame *game, tCommand * command)
{
	int i = command->command_ref;
	int res;

	switch (i)
	{
		case COMMAND_SWEEP:
			res = Sweep(game, command);
			if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
				game->moves--;
			break;
		
		case COMMAND_FLAG:
			if (!(command->flag.is_range))
			{	
				res = DoFlagUnflag(game, command, DO_FLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}	
			else
			{
				res = FlagRange(game, command, DO_FLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves-= res;
			}	
			break;
		
		case COMMAND_UNFLAG:
			if (!(command->flag.is_range))
			{
				res = DoFlagUnflag(game, command, DO_UNFLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}	
			else
			{
				res = FlagRange(game, command, DO_UNFLAG);
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves-= res;
			}	
			break;
		
		case COMMAND_QUERY:
			res = Query(&game->hiddenboard, command);
			//PrintQuery(&command->query);
			//free(command->query.results.results);
			break;

		case COMMAND_SAVE:
			res = WriteSaveFile(game, command->save_filename);
			break;
		
		case COMMAND_QUIT:
			Quit(game, command);
			exit(0);
			break;
		
		case COMMAND_UNDO:
			if (command->undo.can_undo && game->undos)
			{
				Undo(game, &command->undo);
				game->undos--;
				if (game->gametype != GAMETYPE_INDIVIDUAL_NOLIMIT)
					game->moves--;
			}
			else
				printf("%sNo es posible usar undo.%s\n", KERR, KDEF);
			break;


	}

	if (res == SWEEP_MINE && i == COMMAND_SWEEP)
	{
		if (game->undos)
		{	
			if (game->gametype == GAMETYPE_INDIVIDUAL_NOLIMIT)
				AskUndo(game, &command->undo);
			else if (game->moves)
				AskUndo(game, &command->undo);
			else
				game->gamestate = GAMESTATE_LOSE;
		}
		else
			game->gamestate = GAMESTATE_LOSE;
	}	
	return res;
}

void getName(char * name)
{	
	int res = 0;
	char fmt[6];
	sprintf(fmt, "%%%ds", MAX_FILENAME_LEN);
	do
	{
		printf("%sIntroducir nombre de archivo%s\n", KASK, KDEF);
		res = scanf(fmt, name);
	
	} while(!res);

	DELBFF();

	return;

}

void TranslateCoords(tPos * pos)
{
	pos->i = get_row_pos_byref(pos->i);
	pos->j--;
}

static int ValidRow(tPos * pos)
{
	return isupper('A' + pos->i);
}

int CheckLegalPos(tBoard * structboard, tPos * pos)
{
	int res;
	TranslateCoords(pos);
	res = ValidRow(pos);
	res = res && LegalPos(structboard, pos);
	return res;
}

void PrintResult(tGame * game)
{
	switch (game->gamestate)
	{
		case GAMESTATE_WIN:
			PrintBoard(&game->visualboard);
			printf ("%sGanaste!%s\n", KEXC, KDEF);
			if (game->gametype == GAMETYPE_CAMPAIGN)
				game->campaign_level++;
			break;

		case GAMESTATE_CANTWIN:
			printf("%sNo quedan suficientes movimientos para ganar la partida.%s\n", KEXC, KDEF);
			game->gamestate = GAMESTATE_LOSE;
		case GAMESTATE_LOSE:
			PrintBoard(&game->hiddenboard);
			printf("%sPerdiste!%s\n", KEXC, KDEF);
			break;	
	}
}
