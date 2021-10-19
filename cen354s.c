#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>



#define TRUE  0
#define FALSE 1
unsigned long int lineSize     = 0;
unsigned int cmd_args_count    = 0;
unsigned int args_count        = 0;
unsigned int output_method     = 0;
unsigned int redirection_count = 0;
char **args           = NULL;
char **path           = NULL;
char *output_location = NULL;


void first_Init() {
  path = (char**)malloc(2*sizeof(char*));
  path[0] = "/bin/";
  path[1] = NULL;
  
}

void shell_Init(){
  lineSize          = 0;
  args_count        = 0;
  redirection_count = 0;
  output_method     = 0;
  cmd_args_count    = 0;
  args            = NULL;
  output_location = NULL;	
}

char *User_Input(){
  char *usr_input = NULL;
  size_t len = 0;  
  printf("cen354sh> ");
  lineSize = getline(&usr_input, &len, stdin);
 	return usr_input;
}



void Input_Parser(char *input){
  unsigned int input_idx = 0;
  
  while(TRUE){
    if(input[input_idx] == 10){        // ASCII 'New Line'
      break;
    }
    else if(input[input_idx] >= 0x21 && input[input_idx] <= 0x7e){ // In the range of meaningful characters in ASCII
      char *prefix = NULL;
      unsigned int prefix_count = 0;
      prefix = (char*)malloc(sizeof(char));
      prefix[prefix_count] = input[input_idx];
      prefix_count++;
      input_idx++;
      for(input_idx; input[input_idx] >= 0x21 && input[input_idx] <= 0x7e ; input_idx++ ){
        prefix = (char*)realloc(prefix, (prefix_count+1)*sizeof(char));
        prefix[prefix_count] = input[input_idx];
        prefix_count++;
      }
      prefix = (char*)realloc(prefix, (prefix_count+1)*sizeof(char));
      prefix[prefix_count] = '\0';
      
      if(args == NULL){
        args = (char**)malloc(sizeof(char*));
      }else{
        args = (char**)realloc(args, (args_count+1)*sizeof(char*));
      }
      
      args[args_count] = prefix;
      args_count++;
      
    }
    else{  

      input_idx++;
      
    }
    
  }
  args = (char**)realloc(args, (args_count+1)*sizeof(char*));
  args[args_count] = NULL;
  
  return;
}

int countRedirection(){
  
  unsigned int sign_location = 0;
  
  for(int i=0; args[i] != NULL; i++)
    if( strcmp(args[i], ">") == 0){
      redirection_count++;
      sign_location = i;
    }
    
  return sign_location;
}

int redirectionSyntaxCheck(int sign_location){
  int state = FALSE;  // FALSE: There is an error , TRUE: There is no error 
  
  if(redirection_count == 1){     // If there is exactly 1 redirection sign
      if( ((args_count-1) - sign_location) >= 2 ){  // If redirection sign takes multiple arguments
        fprintf(stderr, "Multiple redirection is not allowed\n");
        state = FALSE;
      }
      else if( ((args_count-1) - sign_location) < 1 ){  // If there is no argument after the redirection sign
        fprintf(stderr, "There is no argument for redirection\n");
        state = FALSE;
      }  
      else{
        cmd_args_count = sign_location;
        output_method = 1;
        output_location = strdup(args[sign_location+1]);
        state = TRUE;
      }
  }else if(redirection_count > 1){  // If there is more than 1 redirection sign
    fprintf(stderr, "Multiple redirection sign is not allowed\n");
    state = FALSE;
  }
  else{ // If there is no redirection sign, Do nothing
    state = TRUE;
  }
  
  return state;
}

int ifAPath(char *temp){
    int state = FALSE;
    for(int i=0; temp[i] != '\0'; i++){
      if(temp[i] == '/') state = TRUE;
    }
    if(state == TRUE){
      return TRUE;
    }
    else return FALSE;
}

int checkCdSyntax(){
  int isMultiArgs = FALSE;
  int isNoArgs    = FALSE;

  if( (args_count) > 2) isMultiArgs = TRUE;
  if( (args_count) < 2) isNoArgs    = TRUE;
  
  if(isMultiArgs == FALSE && isNoArgs == FALSE) return TRUE;
  else{
    if(isMultiArgs == TRUE) fprintf(stderr, "cd command can't take multiple parameters\n");
    if(isNoArgs    == TRUE) fprintf(stderr, "cd command needs a parameter\n");
  }
}

int checkPathSyntax(){
  int state = TRUE;
  return state;
}

int syntaxCheck(int isBuiltIn, int sign_location){
  int redState = redirectionSyntaxCheck(sign_location);
  int pathSyntax = TRUE;
  int cdSyntax   = TRUE;
  if (isBuiltIn == 1) cdSyntax   = checkCdSyntax();
  if (isBuiltIn == 2) pathSyntax = checkPathSyntax();
  
  if(redState == TRUE && pathSyntax == TRUE && cdSyntax == TRUE)
    return TRUE;
  else return FALSE;
}

int checkIfBuiltIn(){
  int state = -1;
  if      ( strcmp(args[0], "exit") == 0 ) state = 0;
  else if ( strcmp(args[0], "cd")   == 0 ) state = 1;
  else if ( strcmp(args[0], "path") == 0 ) state = 2;
  else state = -1;
  
  return state;
}

void cd_op(){
    char *directory = args[1];
    int ret;
    ret = chdir (directory);
    if(ret != 0){
    	char *error_message = strdup("An error has occured\n");
    	write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

void path_op(){
  
  path = (char**)malloc(cmd_args_count*sizeof(char*));
  
  int i = 1;
  for(i; i < cmd_args_count; i++){
    char *temp = args[i];
    int k = 0;
    for(k; temp[k] != '\0'; k++);
    if(temp[k-1] != '/') strcat(temp, "/");
    path[i-1] = temp;
  }
  path[i-1] = NULL;
  /***************/
  printf("New Path: ");
  for(int j=0; path[j] != NULL; j++) printf("%s ", path[j]);
  printf("\n");
}

void doBuiltIn( int isBuiltIn ){
  switch(isBuiltIn){
    case 0: // exit
      exit(0);
      break;
    case 1: // cd
      cd_op();
      break;
    case 2: // path
      path_op();
      break;
    default:
      break;
    }
}

void executeCommand(int isAbsolutePath){
  
  char **pass = (char**)malloc((cmd_args_count+1)*sizeof(char*));
  
  
  if(isAbsolutePath == FALSE){
    
    int i = 0;
    int result;
    for(i ; path[i] != NULL; i++){
      char *temp = strdup(path[i]);
      strcat(temp, args[0]);
      result = access(temp, F_OK);
      if(result == 0){
        pass[0] = temp;
        int j = 1;
        for(j; j<cmd_args_count;j++){
          pass[j] = args[j];
        }
        pass[j] = NULL;
        break;
      }
      
    }
    if(result != 0){
        char *error_message = strdup("An error has occured\n");
    	write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
  }else{
    // Check if exists
    int result;
    result = access (args[0], F_OK);
    if(result != 0){
      char *error_message = strdup("An error has occured\n");
    	write(STDERR_FILENO, error_message, strlen(error_message));
      return;
    }else{
      pass[0] = args[0];
      int k = 1;
      for(k; k<cmd_args_count;k++){
        pass[k] = args[k];
      }
      pass[k] = NULL;
    }
  }
  
  
  int rc = fork();
  if ( rc < 0 ){
    fprintf(stderr, "fork failed\n");
  }
  else if ( rc == 0 ){
    if(output_method == 1){

      close(STDOUT_FILENO);
      open(output_location, O_CREAT|O_WRONLY|O_APPEND, S_IRWXU);
    }
    execv( pass[0], pass ); // child: call execv with the path and the args
  }
  else{
    int rc_wait = wait( NULL );
  }
}

int main(int argc, char **argv){
  //system("clear");
    
    first_Init();
    
    if(argc > 0 && argc < 3){

    	if(argc == 1){
    		while(TRUE){
        
        shell_Init();
        
        char *usr_input = User_Input();   // Get user input
        Input_Parser(usr_input);              // Remove the whitespaces and parse the input into arguments
        cmd_args_count = args_count;
        if(args){
          
            int sign_location = countRedirection();
            int isBuiltIn = checkIfBuiltIn();
            int syntaxState = syntaxCheck(isBuiltIn, sign_location);
            if(syntaxState == TRUE){        // Everything is fine, continue!  
              
              if(isBuiltIn >= 0){ // Is built-in
                
                doBuiltIn(isBuiltIn);
                
              }else{              // Is not built-in
                int isAbsolutePath = FALSE;
                if(ifAPath(args[0]) == FALSE) isAbsolutePath = FALSE;
                else isAbsolutePath = TRUE;
                executeCommand(isAbsolutePath);
              }
              
              
            }else{                          // There is a syntax error, do nothing
            
            }
        }else{  }                           // There is no arguments, just pressed to the ENTER , Do nothing         
      }
    }
    else{
      	if(strcmp(argv[1], "--help") == 0){
      		printf("This is a shell implementation for school assignment called cen354sh\n");
      		printf("Cen354sh has 3 built-in command:\n--> cd : Changes the directory\n--> path : Changes the shell path (Overwrites the current one)\n");
      		printf("--> exit : Exits from shell with 0 error code\n");
      		printf("\nAlso redirection (>) can be used within cen354sh. But consider that there must be a space between redirection sign and its argument.\n\n");
      	}
      	else printf("Unrecognized argument\n");
      }
    
    }else{
    	char *error_message= strdup("Shell can be called with only one or none parameter\n");
      write(STDERR_FILENO, error_message, strlen(error_message));
    }     
      

  
  return 0;                                 // Exit without error
  
}




