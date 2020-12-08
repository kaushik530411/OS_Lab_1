#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
char *token;
pid_t pid;

// Handlers
void int_handler(int signum){printf("\n");kill(pid, SIGKILL);}
void timed_handler(int signum){printf("\n");kill(pid, SIGKILL);}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];
  
    // Stores the current working directory. 
    char curr_working_dir[MAX_COMMAND_LINE_LEN];
  
    while (true) {
        getcwd(curr_working_dir, sizeof(curr_working_dir));
        do{ 
            // Print the shell prompt.
            printf("%s%s", curr_working_dir, prompt);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        } while(command_line[0] == 0x0A);  // while just ENTER pressed

      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        // TASK 0. Modify the prompt to print the current working directory
         // TASK 1. Tokenize the command line input (split it on whitespace)
         int index = 0;
         token = strtok(command_line, delimiters);
         while (token != NULL) {
           arguments[index] = token;
           index++;
           token = strtok(NULL, delimiters);
         }
         
         // TASK 2. Implement Built-In Commands
         if (strcmp(arguments[0], "pwd") == 0){  
            printf("%s\n", curr_working_dir);
            continue;
         } else if (strcmp(arguments[0], "echo") == 0) {
            char output[MAX_COMMAND_LINE_LEN];
            int outputIndex = 1;
            while (outputIndex < index) {
              if (outputIndex > 1){
                 strcat(output, " ");
              }
              if (arguments[outputIndex][0] == '$') {
                char* pathVariable;
                char *temp = strtok(arguments[outputIndex], "$"); 
                pathVariable = getenv(temp);
                if (pathVariable != NULL)
                  strcat(output, pathVariable);
              } else {
                strcat(output, arguments[outputIndex]);
              }
              outputIndex += 1;
            }
            printf("%s\n", output);
            strcpy(output, "");
            continue;
         } else if (strcmp(arguments[0], "exit") == 0){
            break;
         } else if (strcmp(arguments[0], "cd") == 0){
           chdir(arguments[1]);
           continue;
         } else if (strcmp(arguments[0], "setenv") == 0){
           char *token = strtok(arguments[1], "="); 
           char *envVar = token;
           char *envVal;
            while (token != NULL) {
                envVal = token; 
                token = strtok(NULL, "="); 
            } 
           setenv(envVar, envVal, 1);
           continue;
         } else if (strcmp(arguments[0], "env") == 0){
           char **env = environ;
           while (*env != 0) {
              char *thisEnv = *env;
              printf("%s\n", thisEnv);
              env++;
            }
           continue;
         }

         // Task 3. Create a child process which will execute the command line input
          pid = fork();  
          int backgroundProcess = 0;
	  if (arguments[1] != NULL){
		if (strcmp(arguments[1], "&") == 0){
			backgroundProcess = 1;
			arguments[1] = NULL;
		}
	  }
        
        if (pid < 0) {  
            perror("Fork error!\n");
            exit(1);
        } else if (pid == 0) {
            // TASK 4. The parent process should wait for the child to complete unless its a background process
            signal(SIGINT, int_handler);  
        
        int fd0;  // Task 6 - Extra Credit 
	    int outputTrigger = 0;
	     char output[64];
            int j; 
	    for(j=0; arguments[j] != '\0'; j++)
	    {
		if(strcmp(arguments[j], ">")==0){        
				arguments[j]=NULL;
				strcpy(output, arguments[j+1]);
				outputTrigger=1;           
		}                       
	    }					
	    if(outputTrigger > 0)
	    {
		int fd1 ;
		if ((fd1 = creat(output , 0644)) < 0) {
				perror("Error occurred while trying to open the output file");
				exit(0);
		}           

		dup2(fd1, 1);
		close(fd1);
	    }
            
            if (execvp(arguments[0], arguments) < 0 ){
              perror("execvp() failed: No such file or directory\n");   // Input is not executable.
              exit(1); 
            }
            exit(0);
          } else {
            signal(SIGINT, int_handler); // Task 4 - Signal Handling
            signal(SIGALRM, timed_handler); // Task 5 - Killing off long processes 
            alarm(10);
            if (backgroundProcess == 1){
              backgroundProcess = 0;
            } else {
              wait(NULL);
            }
          }

    }
    // This should never be reached.
    return -1;
}
