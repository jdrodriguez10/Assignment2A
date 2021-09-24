#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
extern char **environ;


void func_Exit(){
  exit(0);
}


void func_Env(){
  int i;
  for (i = 0; environ[i] != NULL; i++)
    printf("%s\n",environ[i]);
}


void func_Clear(){
  printf("\e[1;1H\e[2J");
}


void func_Dir(char **tokens){
  char a[150]="ls -la ";
  system(strcat(a,tokens[1]));
}


void func_Pwd(){
  char *tem_env = (char *)malloc(sizeof(char)*150);
  getcwd(tem_env,150);
  printf("%s:$ ",tem_env);
  free(tem_env);
}


void func_Cd(char *fpath){
  char *tem_env = (char *)malloc(sizeof(char)*150);
  getcwd(tem_env,150);

  if(fpath && chdir(fpath)!=0){
    printf("ERROR: Directory doesn't exist!\n");
    return;
  }
  printf("BEFORE:\nOLDPWD=%s\nPWD=%s\n",getenv("OLDPWD"),getenv("PWD"));
  if(fpath){
    setenv("OLDPWD",tem_env,1);
    getcwd(tem_env,150);
    setenv("PWD",tem_env,1);
  }
  else
    setenv("OLDPWD",tem_env,1);

  printf("AFTER:\nOLDPWD=%s\nPWD=%s\n",getenv("OLDPWD"),getenv("PWD"));
  free(tem_env);
}


void io_Redirect(char **tokens,int n){
  FILE *f_p;
  char **arg_list = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  int j=0;

  for(int i=0;strcmp(tokens[i],">>") && strcmp(tokens[i],">") && strcmp(tokens[i],"<");i++){
    arg_list[j] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
    strcpy(arg_list[j++],tokens[i]);
  }
  arg_list[j]=NULL;
  char *tem_env = (char *)malloc(sizeof(char)*150);

  switch (fork()) {
  case -1:
    break;
  case 0:
    if(!strcmp(tokens[n-2],">>"))
      f_p = freopen(tokens[n-1],"a",stdout);
    else if(!strcmp(tokens[n-2],">"))
      f_p = freopen(tokens[n-1],"w",stdout);
    else
      f_p = freopen(tokens[n-1],"r",stdin);
    strcpy(tem_env,"/bin/");
    strcat(tem_env,arg_list[0]);
    if(execv(tem_env,arg_list)==-1){
      strcpy(tem_env,"/usr/bin/");
      strcat(tem_env,arg_list[0]);
      execv(tem_env,arg_list);
    }
    free(tem_env);
  }
  wait(NULL);

  for(int i=0;arg_list[i]!=NULL;i++)
    free(arg_list[i]);

  free(arg_list);
}


/* Splits the string by space and returns the array of tokens
 */
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0;
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
  char  line[MAX_INPUT_SIZE];
  char  **tokens;
  int i;
  clock_t t_1;
  char error_message[30] = "An error has occured\n";

  FILE* f_p;
  if(argc == 2) {
    f_p = fopen(argv[1],"r");
    if(f_p < 0) {
      write(STDERR_FILENO, error_message, strlen(error_message));
      return -1;
    }
  }

  while(1) {
    int prl=0,srl=0,bg=0,i_o=0,c_dir=0;

    /* BEGIN: TAKING INPUT */
    bzero(line, sizeof(line));
    if(argc == 2) {
      if(fgets(line, sizeof(line), f_p) == NULL) {
	break;
      }
      line[strlen(line) - 1] = '\0';
    } else {
      func_Pwd();
      // waitpid(-1,NULL,WNOHANG);
      scanf("%[^\n]", line);
      getchar();
    }
    if(!strlen(line))
      continue;
    // printf("Command entered: %s (remove this debug output later)\n", line);
    /* END: TAKING INPUT */

    line[strlen(line)] = '\n';
    tokens = tokenize(line);

    //do whatever you want with the commands, here we just print them
    for(i=0;tokens[i]!=NULL;i++){
      if(!i_o)
	i_o=strcmp(tokens[i],">>")==0 || strcmp(tokens[i],">")==0 || strcmp(tokens[i],"<")==0;

      // printf("found token %s (remove this debug output later)\n", tokens[i]);
    }
    t_1 = clock();
    if(i_o)
      io_Redirect(tokens,i);
    else {
      if(!strcmp(tokens[0],"dir")){
	func_Dir(tokens);
      }
      else if(!strcmp(tokens[0],"clear")){
	func_Clear();
      }
      else if(!strcmp(tokens[0],"quit")){
	func_Exit();
      }
      else if(!strcmp(tokens[0],"env")){
	func_Env();
      }
      else if(!strcmp(tokens[0],"cd")){
	func_Cd(tokens[1]);
      }
      else{
	system(line);
      }
    }

    t_1 = clock() - t_1;
    printf("%f sec\n",(double)t_1/CLOCKS_PER_SEC);

    // Freeing the allocated memory
    for(i=0;tokens[i]!=NULL;i++){
      free(tokens[i]);
    }
    free(tokens);
  }
  return 0;
}
