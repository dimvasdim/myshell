/* A project implemented for university course
  Operating Systems at 7th semester, Aristotle
  University of Thessaloniki, Greece, 07/01/2019
  by Vasileios Dimitriadis 8404 */


#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  LINESIZE   512 // This the maximum number of characters in a line.
#define  TOK_DELIM  " \t\r\n\a"


/* Shell modes are the interactive and the batch mode.
  In interactive mode the user can type commands that
  wants to be executed using the prompt. In batch mode
  the user should provide a batchfile that has commands
  written in it and all of these commands will be executed.
*/
void interactive_mode();
void batch_mode(char const *batchfileName);


// Functions
void prompt();
char *getLine();
char **getArgs(char *line);
int simpleExecute(char **args, int waitCmd);
int execute(char **args);
char *getFileLine(FILE *fp);
int myshell_cd(char **args);
int myshell_help(char **args);



int main(int argc, char const *argv[])
{
  // Check number of arguments to decide in which mode to run
  if(argc == 1)
  {
    // Run shell in Interactive mode
    interactive_mode();
  }
  else if(argc == 2)
  {
    // Run shell in Batch mode
    batch_mode(argv[1]);
  }
  else
  {
    // Incorrect number of arguments
    printf("Enter the correct number of arguments...\n");
    printf("Enter no arguments to run shell in interactive mode\n");
    printf("OR enter the name of a batchfile to run shell in batch mode!!!\n");
  }
  return 0;
}


/* This shell has some built-in commands that need to
  be implemented differently than the regular ones that
  are already functions of Linux. These two built-in
  commands are 'cd' that is used to change directory and
  'help' that is used to provide info about myshell.
*/
int (*builtin_func[]) (char **) = {&myshell_cd, &myshell_help};


char *builtin_str[] = {"cd", "help"};


/* The following function is used to calculate the
  number of built-in commands of myshell as there
  is a possibility to extend them. */
int num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}


int myshell_cd(char **args)
{
  if (args[1] == NULL)
  {
    printf("Expected argument to \"cd\"...\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    if(chdir(args[1]) != 0)
    {
      printf("Error using cd command");
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}


int myshell_help(char **args)
{
  int i;
  printf("This is a new shell!\n");
  printf("Type commands and arguments and hit enter.\n");
  printf("The followings are builtin:\n");
  for (i=0; i<num_builtins(); i++)
  {
    printf("%s  ", builtin_str[i]);
  }
  printf("\n");
  printf("To exit this shell type 'quit'\n");
  return 0;
}


// This is what is being displayed on the prompt.
void prompt()
{
  printf("dimitriadis_8404> ");
}


// Read the line that there is at prompt.
char *getLine()
{
  int bufsize = LINESIZE;
  char *line = malloc(bufsize * sizeof(char));

  if(line == NULL)
  {
    printf("Allocation error in function getLine...\n");
    exit(EXIT_FAILURE);
  }

  fflush(stdin);
  if (fgets(line, bufsize, stdin) == NULL)
  {
    printf("Can't read more than %d characters at one line...\n", bufsize);
    exit(EXIT_FAILURE);
  }
  return line;
}


// Split the line into arguments as to manage the commands
// that are given more easily.
char **getArgs(char *line)
{
  int bufsize = LINESIZE;
  int index = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if(tokens == NULL)
  {
    printf("Can't allocate memory for tokens...\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);
  while(token != NULL)
  {
    tokens[index] = token;
    index++;
    token = strtok(NULL, TOK_DELIM);
  }
  tokens[index] = NULL;
  return tokens;
}


/* This is the function that executes all the commands that
  are given. It forks the process and the child process is
  the one that executes the process while the parent process
  is waiting for it.
*/
int simpleExecute(char **args, int waitCmd)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) // Child process
  {
    if (execvp(args[0], args) == -1)
    {
      printf("Invalid argument \'%s\'.\n", args[0]);
      exit(EXIT_FAILURE);
    }
    else
    {
      return 0;
    }
  }
  else if (pid < 0) // Error forking
  {
    printf("Error forking...\n");
    exit(EXIT_FAILURE);
  }
  else // Parent process
  {
    do
    {
      wpid = waitpid(pid, &status, WUNTRACED);
      if (wpid == -1)
      {
          printf("Error in waiting pid...\n");
          exit(EXIT_FAILURE);
      }
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));

    if (waitCmd == 1)
    {
      if (WEXITSTATUS(status) == 0)
      {
        return 0;
      }
      else
      {
        printf("Arguments following \'%s\' aren't executed...\n", args[0]);
        return EXIT_FAILURE;
      }
    }
    else if (waitCmd == 0)
    {
        return 0;  // Don't care if previous command executed successfully or not.
    }
    else
    {
        printf("Invalid waitCmd value:%d...\n", waitCmd);
        exit(EXIT_FAILURE);
    }
  }
  return 0;
}


int execute(char **args)
{
  int i = 0;
  int waitCmd = 0;
  int j, status, no_builtin;
  char **simpleArgs = malloc(LINESIZE * sizeof(char*));

  if(simpleArgs == NULL)
  {
    printf("Can't allocate memory for simpleArgs...\n");
    exit(EXIT_FAILURE);
  }

  while (args[i] != NULL)
  {
    // first check if user wants to quit our shell.
    if (strcmp(args[i], "quit") == 0)
    {
      printf("You are exiting this shell...\n");
      printf("Thanks for using! Goodbye!!!\n");
      free(simpleArgs);
      return -1;
    }
    // if there are multiple user commands
    // split them into single commands and then execute them.
    j = 0;
    do
    {
      simpleArgs[j] = strdup(args[i]);
      j++;
      i++;
    }while(args[i] != NULL && strcmp(args[i], ";") != 0 && strcmp(args[i], "&&") != 0);
    simpleArgs[j] = NULL;

    if (args[i] != NULL)
    {
      if (strcmp(args[i], ";") == 0)
      {
        waitCmd = 0;    // don't wait for the previous command to finish
      }
      if (strcmp(args[i], "&&") == 0)
      {
        waitCmd = 1;   // wait for previous command to finish and execute only if returned successfully
      }
      i++;
    }

    no_builtin = 1; // This is used as a way to know if the command is built-in or regular one.
    // We check if the command is a built-in so as to be executed in this stage
    // and not to be forked and executed by a child process.
    for(j = 0; j < num_builtins(); j++)
    {
      if(strcmp(simpleArgs[0], builtin_str[j]) == 0)
      {
        no_builtin = 0; // so we know it is a built-in
        status = (*builtin_func[j])(simpleArgs);
      }
    }

    if(no_builtin)
    {
      status = simpleExecute(simpleArgs, waitCmd);
    }

    // free allocated memory
    j = 0;
    while (simpleArgs[j] != NULL) {
      free(simpleArgs[j++]);
    }

    if (status != 0)
    {
      break;
    }
  }
  // Free memory
  free(simpleArgs);
  return 0;
}


void interactive_mode()
{
  char *line;
  char **args;
  int status;

  do
  {
    prompt();
    line = getLine();
    args = getArgs(line);
    status = execute(args);
    free(line);
    free(args);
  }while(status != -1);
}


// Read the line of a file.
char *getFileLine(FILE *fp)
{
  int bufsize = LINESIZE;
  char *line = malloc(bufsize * sizeof(char));

  if(line == NULL)
  {
    printf("Allocation error in function getFileLine...\n");
    exit(EXIT_FAILURE);
  }

  if(fgets(line, bufsize, fp) == NULL)
  {
    if(feof(fp))
    {
      strcpy(line, "quit");
    }
    else
    {
      printf("Can't read more than %d characters in a line...\n", bufsize);
      exit(EXIT_FAILURE);
    }
  }
  fflush(fp);
  return line;
}


void batch_mode(char const *batchfileName)
{
  FILE *fp;
  char *line;
  char **args;
  int status;

  prompt();
  printf("Executing commands in file '%s' line by line\n", batchfileName);
  fflush(stdout);

  fp = fopen(batchfileName, "r");
  if(fp == NULL)
  {
    printf("Can't open batchfile %s...\n", batchfileName);
    exit(EXIT_FAILURE);
  }

  do
  {
    printf("\n");
    line = getFileLine(fp);
    args = getArgs(line);
    status = execute(args);
    free(line);
    free(args);
  }while(status != -1);

  fclose(fp);
}
