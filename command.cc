
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctime>

#include "command.h"

char *str;

// Log File Variables
char *pathLogFile = (char*)malloc(100*sizeof(char));
char currentTime[32];
FILE *logFile;
time_t TIMER = time(NULL);
tm *ptm = localtime((&TIMER));

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_concatenate = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_concatenate = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
		printf("\n");
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	
	// Print contents of Command data structure
	print();
	
	// Default input, output and error directories
	int defIn = dup(0);
	int defOut = dup(1);
	int defErr = dup(2);
	
	int piping[_numberOfSimpleCommands][2];
	
	// Input, Output and Error files to be created if needed
	int input, output, error;
	
	if(_errFile)
	{
		error = open(_errFile, O_WRONLY | O_CREAT, 0666);
		dup2(error, 2);
	}
	if(_inputFile)
	{
		input = open(_inputFile, O_RDONLY, 0666);
	}
	if(_outFile)
	{
		if(_concatenate == 0)
		{
			// Write in file (create it if not existing or empty it before writing if exists)
			output = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		}
		else
		{
			// Append in file (write or append if exists or create if not existing)
			output = open(_outFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
		}
	}
	
	for(int i = 0; i < _numberOfSimpleCommands; i++)
	{
		pipe(piping[i]);

		// Setting input, output and error constraints (default, files or piped)
		if(i == 0)
		{
			if(_inputFile)
			{
				dup2(input, 0);
				close(input);
			}
			else
			{
				dup2(defIn, 0);
			}
		}
		else
		{
			dup2(piping[i-1][0], 0);
			close(piping[i-1][0]);
		}
		
		if(i == _numberOfSimpleCommands-1)
		{
			if(_outFile)
			{
				dup2(output, 1);
				close(output);
			}
			else
			{
				dup2(defOut, 1);
			}
		}
		else
		{
			dup2(piping[i][1], 1);
			close(piping[i][1]);
		}

		// Checking if cd command is called
		if(strcmp(_simpleCommands[i]->_arguments[0], "cd") == 0)
		{
			char *directory;
			// Check if the directory to go to is the upper directory
			if(_simpleCommands[i]->_numberOfArguments == 1)
			{
				char *buf = (char *)malloc(100*sizeof(char));
				getcwd(buf, 100);
				int j = 0;
				int back = 0;
				while(j < strlen(buf))
				{
					if(buf[j] == '/')
					{
						back++;
					}
					if(back == 3)
					{
						break;
					}
					j++;
				}
				buf[j] = '*';
				directory = strtok(buf, "*");
				
			}
			else if(strcmp(_simpleCommands[i]->_arguments[1], "..") == 0)
			{
				char *buf = (char *)malloc(100*sizeof(char));
				getcwd(buf, 100);
				int j = strlen(buf)-1;
				while(buf[j] != '/')
				{
					j--;
				}
				buf[j] = '*';
				directory = strtok(buf, "*");
			}
			else
			{
				directory = _simpleCommands[i]->_arguments[1];
				if(directory[0] == '/')
				{
					for(int i = 0; i < strlen(directory)-1; i++)
					{
						directory[i] = directory[i+1];
					}
					directory[strlen(directory)-1] = NULL;
				}
			}
			int temp = chdir(directory);
			if(temp < 0)
			{
				perror("");
			}
			str = _simpleCommands[i]->_arguments[0];
			signal(SIGCHLD, logging);
			break;
		}

		// Fork
		int processId = fork();
		
		if(processId == 0)
		{
			execvp(_simpleCommands[i]->_arguments[0], &_simpleCommands[i]->_arguments[0]);
		}
		else
		{
			str = _simpleCommands[i]->_arguments[0];
			signal(SIGCHLD, logging);
			// Restoring the default input, output and error
			dup2(defIn, 0);
			dup2(defOut, 1);
			dup2(defErr, 2);
			if (_background == 0)
			{	
				waitpid(processId, 0, 0);
			}
		}
	}

	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

// Canceling efect of Ctrl+C
void terminateC(int sig)
{
	signal(SIGINT, terminateC);
	Command::_currentCommand.clear();
	printf("\r\033[0J");
	Command::_currentCommand.prompt();
	fflush(stdout);
}

// Writting in Log File
void logging(int sig)
{
	int stat;
	wait(&stat);
	logFile = fopen(pathLogFile, "a");
	flockfile(logFile);
	strcpy(currentTime, asctime(ptm));
	for (int i = 0; i < strlen(currentTime); i++)
	{
		if (currentTime[i] == '\n')
		{
			currentTime[i] = '\0';
			break;
		}
	}
	fprintf(logFile, "%s: %s command terminated\n", currentTime, str);
	funlockfile(logFile);
	fclose(logFile);
	signal(SIGCHLD, logging);
}

void
Command::prompt()
{
	signal(SIGINT, terminateC);
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	getcwd(pathLogFile, 100);
	strcat(pathLogFile, "/log.txt");
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

