// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include<signal.h>
#include<errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

//Global history commands array
#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH];
int tracker=0;
int totalCmd=0;

/**
 * Command Input and Processing
 */

void write_history(char* tokens[],bool in_background){

  if ((tokens[0] != NULL) && (strlen(tokens[0]) != 0)){

    char temp[COMMAND_LENGTH] = "";

    bool flag = false;
    for (int i = 0; tokens[i] != NULL; i++) {
      if (flag){
        strcat(temp, " ");
      }
      strcat(temp, tokens[i]);
      flag = true;
    }

    if(in_background){
      strcat(temp, " &");
    }

    memset(&history[tracker], 0, COMMAND_LENGTH);
    memcpy(&history[tracker][0], temp, strlen(temp)+1 );

    totalCmd++;tracker++;
  
    if (tracker > 9) tracker = 0;

  }
}

void print_history(){

  int index = 0;
  if (totalCmd > 9){
    index = totalCmd%10;
  }

  for(int i=0 ; (i<10 && strcmp(history[i],"")!= 0); i++){
    char buff[2];
    int temp = i;
    if( totalCmd > 10 ){
      sprintf(buff, "%d\t", (totalCmd - (9-temp)) );
    }else {
      sprintf(buff, "%d\t", temp+1 );
    }
    write(STDOUT_FILENO, buff, strlen(buff));
    write(STDOUT_FILENO, (history[index]), strlen( (history[index]) ));
    write(STDOUT_FILENO, "\n", strlen("\n"));

    index++;
    index = index%10;

  }

}

void init_history(){
  memset(history, 0, sizeof(history[HISTORY_DEPTH][COMMAND_LENGTH]) * HISTORY_DEPTH * COMMAND_LENGTH);
}

void handle_SIGINT(){
  write(STDOUT_FILENO, "\n", strlen("\n"));
  print_history();
}


/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
  int token_count = 0;
  _Bool in_token = false;
  int num_chars = strnlen(buff, COMMAND_LENGTH);
  for (int i = 0; i < num_chars; i++) {
    switch (buff[i]) {
    // Handle token delimiters (ends):
    case ' ':
    case '\t':
    case '\n':
      buff[i] = '\0';
      in_token = false;
      break;

    // Handle other characters (may be start)
    default:
      if (!in_token) {
        tokens[token_count] = &buff[i];
        token_count++;
        in_token = true;
      }
    }
  }
  tokens[token_count] = NULL;
  return token_count;
}


void read_command(char *buff, char *tokens[], _Bool *in_background)
{
  *in_background = false;

  memset(tokens, '\0', COMMAND_LENGTH);

  // Read input
  int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

  if ( (length < 0) && (errno != EINTR) ){
    perror("Unable to read command. Terminating.\n");
    exit(-1);  /* terminate with error */
  }

  // Null terminate and strip \n.
  buff[length] = '\0';
  if (buff[strlen(buff) - 1] == '\n') {
    buff[strlen(buff) - 1] = '\0';
  }

  // Tokenize (saving original command string)
  int token_count = tokenize_command(buff, tokens);
  if (token_count == 0) {
    return;
  }

  // Extract if running in background:
  if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
    *in_background = true;
    tokens[token_count - 1] = 0;
  }
}

/**
 * Main and Execute Commands
 */


int main(int argc, char* argv[])
{
  char input_buffer[COMMAND_LENGTH];
  char *tokens[NUM_TOKENS];
  memset(tokens, 0, COMMAND_LENGTH);

  char* dir;
  init_history();

  struct sigaction handler;
  handler.sa_handler = handle_SIGINT;
  handler.sa_flags= 0;
  sigemptyset(&handler.sa_mask);
  sigaction(SIGINT, &handler, NULL);

  while (true) {

    // Get command
    // Use write because we need to use read() to work with
    // signals, and read() is incompatible with printf().
    dir = getcwd(NULL, 0);
    write(STDOUT_FILENO, dir, strlen(dir));
    write(STDOUT_FILENO, "> ", 2);
    _Bool in_background = false;
    input_buffer[0] = '\0';
    read_command(input_buffer, tokens, &in_background);

    //------------------------Handle internal commands-----------------------------------------------
    if(tokens[0] != NULL && (strlen(tokens[0]) != 0)  && strcmp(tokens[0],"") != 0 ){

      //-----------------__Handle history commands-------------------------------------------------
      if (strcmp(tokens[0], "!!")==0){
        if ( ((totalCmd-1)%10) < 0 && strcmp(history[(totalCmd-1)%10],"")==0  ){
           write(STDIN_FILENO, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
           continue;
        }
        else {
          write(STDOUT_FILENO, history[(totalCmd-1)%10], strlen(history[(totalCmd-1)%10]) );
          write(STDIN_FILENO, "\n", strlen("\n"));
          
          char temp[COMMAND_LENGTH];
          strcpy(temp,  history[(totalCmd-1)%10]);
          int token_count = tokenize_command(temp, tokens);
          // Extract if running in background:
          if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
            in_background = true;
            tokens[token_count - 1] = 0;
          }

        }
      }

      if (strchr(tokens[0], '!')){ 
        int i, dec=0,len=0;
        bool flag=true;
        len = strlen(tokens[0]);

        for(i=1; i<len; i++){
          if (isdigit(tokens[0][i])){
            dec = dec * 10 + ( tokens[0][i] - '0' );
          }else {
            flag = false;
          }
        }
        if(dec < (totalCmd-9) || dec > totalCmd || strcmp(history[(dec-1)%10],"") == 0 || dec <= 0) flag = false;

        if (flag){
          write(STDIN_FILENO, history[(dec-1)%10], strlen(history[(dec-1)%10]) ) ;
          write(STDIN_FILENO, "\n", strlen("\n"));
          char temp[COMMAND_LENGTH];
          strcpy(temp, history[(dec-1)%10]);
          int token_count = tokenize_command(temp, tokens);
          // Extract if running in background:
          if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
            in_background = true;
            tokens[token_count - 1] = 0;
          }

        }else {
          write(STDIN_FILENO, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
          continue;
        }
      }
      //-----------------______END history command_______--------------------------------------------
      
      //Error check -> If !! or !n command then write to history. 
      if (strcmp(tokens[0], "!!") != 0  && strcmp(tokens[0], "!n") != 0) write_history(tokens, in_background);
        
      //Exit out if if exit command recived  
      if (strcmp(tokens[0], "exit") == 0 ) return 0;

      //handle pwd command 
      if (strcmp(tokens[0], "pwd") == 0){
        dir = getcwd(NULL, 0);
        write(STDOUT_FILENO, dir, strlen(dir));
        write(STDOUT_FILENO, "\n", 1);
        continue;

      //handle cd command using chdir();
      }else if(strcmp(tokens[0], "cd") == 0){
        int error;
    
        error = chdir(tokens[1]);
        if (error != 0 ){
          write(STDOUT_FILENO, "Invalid directory.\n", strlen("Invalid directory.\n"));
        }
        continue;

      // Handle history command
      }else if( strcmp(tokens[0], "history") == 0 ){
        print_history();
        continue;
      }
    
    
      /**
      * Steps For Basic Shell:
      * 1. Fork a child process
      * 2. Child process invokes execvp() using results in token array.
      * 3. If in_background is false, parent waits for
      *    child to finish. Otherwise, parent loops back to
      *    read_command() again immediately.
      */
      //--------------------------------------HANDLE CHILD PROCESS -------------------------------------------------------
      pid_t child;
      child = fork();
  
      if (child == -1){
        write(STDOUT_FILENO, "   Error creating frok!! ", strlen("   Error creating frok!! "));
        exit(0);
      }else if( child == 0 ){
        if (execvp(tokens[0], tokens) < 0) {
          write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
          write(STDOUT_FILENO, ": Unknown command.\n", strlen(": Unknown command.\n"));
        }
        exit(-1);
      }else {
        if (!in_background){
          waitpid(child, NULL,0) ;
        }
      }
      while (waitpid(-1, NULL, WNOHANG) > 0);
  
      //--------------------------------------HANDLE CHILD PROCESS END -------------------------------------------------------
       
    }
    
  }
  return 0;
}
