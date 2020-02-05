/***********************************
 * Author: Ibrahim Alarifi
 * Date: 08/05/2019
 * Discreption: A basic shell that connect the user with the OS, it has three built in commands (cd, exit and status)
 *      it can also perform all the bash commands.
 * Output: User interface with the shell
 * Input: The user's commands
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h> 
#include <unistd.h>
#include <time.h>
#include <signal.h>

int exitStatus=-5; // holds the previous exit status
int termSignal=-5; // hold the previous termination code
int TSTPcounter=0; // Used to implement SIGTSTP behavior

struct sigaction SIGINT_action = {0}; // SIGINT
struct sigaction SIGTSTP_action = {0}; // SIGTSTP
struct sigaction ignore_action = {0}; // ignore
sigset_t TSTPsig; // hold SIGTSTP only in that set

// declare the cmmdLine struct, in which input will be read
struct cmmdLine
{
   char command[20];
   char args[513][20];
   char inputfile[20];
   char outputfile[20];
   int flags[4]; // 0: #args. 1: input. 2: output. 3: background

};

void bgCheck(int pids[]); // declaring function


/***********************************
 * This function is the handler for SIGTSTP signal withing the main shell
 * Input: int signo
 * Output: Outputs the running mode to the terminal via stdout
 **********************************/
void catchSIGTSTP(int signo)
{
   if(TSTPcounter==0){  // if background is enable 
      char* message ="\nEntering foreground-only mode (& is now ignored)\n"; // print this message
      write(STDOUT_FILENO, message, 50);
      TSTPcounter=1; // disable background
   }
   else if(TSTPcounter==1){ // if background is disable
      char* message = "\nExiting foreground-only mode\n"; // print this message
      write(STDOUT_FILENO, message, 30);
      TSTPcounter=0; // enable background
   }
}

/***********************************
 * This function interact with the user to get the input from them, then returns it
 * Input: list of running PIDs
 * Output: Returns a string input after validating its correctness
 **********************************/
char* getInput(int myPIDs[])
{
   char* readBuff=NULL;
   size_t buffSize=0;
   int letters;
   int i, checkCount=0;
   int printCount=0;

   while(1){ // loops until it gets a proper input 

      bgCheck(myPIDs); // checks if any background command has exited
      while(1)
      {
	 printf(": "); // ask the user to input data
	 fflush(stdout);
	 letters = getline(&readBuff, &buffSize, stdin); // reads input from stdin to readBuff
	 if (letters == -1){ // if it gets interrupted and fails
	    clearerr(stdin);
	 }
	 else{ // if it succeed 
	    readBuff[letters - 1] = '\0'; // remove the new line char from the input 
	    break; // Exit the loop - we've got input
	 }
      }
      if(readBuff[0] == '#'); // if input is a comment
      else if(readBuff[0]=='\0'); // if input is blank
      else // actual command
	 return readBuff;
   } // loops again 
}

/***********************************
 * This function clears the content of the cmmdLine struct
 * Input: cmmdLine struct
 * Output: -  
 **********************************/
void clearCmmdLine(struct cmmdLine* myst)
{
   int i=0;
   memset(myst->command, '\0', sizeof(myst->command)); // clears the command
   memset(myst->inputfile, '\0', sizeof(myst->inputfile)); // clears inputfile name
   memset(myst->outputfile, '\0', sizeof(myst->outputfile)); // clears outpufile name
   for (i=0; i<4;i++)
      myst->flags[i]=0; // this clears the flags

   for(i=0; i<513; i++){ // this loop clears the args with null char
      memset(myst->args[i], '\0', sizeof(myst->args[i]));
   }
}

/***********************************
 * This function process the input string to the commandline struct
 * Input: cmmdLine struct, input string
 * Output: -  
 **********************************/
void processInput(struct cmmdLine* myst, char* input)
{
   char temp[2048];
   char buff[20];
   char dBuff[20];
   char delim[] = " ";
   int i, j, argCheck, index=1;
   memset(temp, '\0', sizeof(temp)); // clears temp
   strcpy(temp, input); // copy the input to temp
   char *ptr = strtok(temp, delim); // strtok and get the command
   strcpy(myst->command, ptr); // store it to the struct
   strcpy(myst->args[0], myst->command); // store it in arg[0] as well 
   myst->flags[0]=index; // increment # of args

   argCheck = 1; // arguement counter ( set if argument)
   do{ // loops to strtok the rest of the input
      char* p=NULL;
      ptr = strtok(NULL, delim); // strtok the next word
      if(ptr == NULL){ // end of input
	 if ((input[strlen(input) - 1]=='&') && (TSTPcounter == 0)){ // if background command is provided
	    myst->flags[3]=1; // store it to the flags at [3]
	 }
	 return;
      }
      else if(*ptr=='<'){ // if input file is provided
	 ptr = strtok(NULL, delim);
	 memset(buff, '\0', sizeof(buff)); // clears buff
	 strcpy(buff,ptr);
	 p = strstr(ptr, "$$");  // if it has $$
	 if (p){ // if there is actually $$
	    memset(buff, '\0', sizeof(buff)); // clears buff
	    for(i=0; i<strlen(ptr);i++){
	       if(ptr[i] == *p){ // replace the 1st $ with PID
	          int x = getpid(); // get the pid
	          sprintf(dBuff, "%d", x); // expand it
		  strcat(buff,dBuff); // strcat it to buff
		  i++; // skips the next $
	       }
	       else{ // copy the next letter
		  int len = strlen(buff);
		  buff[len]=ptr[i]; // adds the next char to the buff string
	       }
	    }
	 }
	 strcpy(myst->inputfile, buff); // store it to the struct
	 argCheck = 0; // clear arg counter
	 myst->flags[1]=1; // set input flag
      }
      else if (*ptr=='>'){ // if output file is provided
	 ptr = strtok(NULL, delim);
	 memset(buff, '\0', sizeof(buff)); // clears buff
	 strcpy(buff,ptr);
	 p = strstr(ptr, "$$"); // if it has $$
	 if (p){ // if there is actually $$
	    memset(buff, '\0', sizeof(buff)); // clears buff
	    for(i=0; i<strlen(ptr);i++){
	       if(ptr[i] == *p){ // replace the 1st $ with PID
	          int x = getpid(); // get the pid
	          sprintf(dBuff, "%d", x); // expand it
		  strcat(buff,dBuff); // strcat it to buff
		  i++; // skips the next $
	       }
	       else{ // copy the next letter
		  int len = strlen(buff);
		  buff[len]=ptr[i]; // adds the next char to the buff string
	       }
	    }
	 }
	 strcpy(myst->outputfile, buff); // store it to the struct
	 argCheck = 0; // clear arg counter
	 myst->flags[2]=1; // set output flag
      } 

      else if ((argCheck==1)&&(strcmp(ptr, "&") != 0)){ // if it's an arg
	 memset(buff, '\0', sizeof(buff)); // clears buff
	 strcpy(buff, ptr);
	 p = strstr(ptr, "$$"); // this is for PID ($$) expansion 
	 if (p){ // if there is actually $$
	    memset(buff, '\0', sizeof(buff)); // clears buff
	    for(i=0; i<strlen(ptr);i++){
	       if(ptr[i] == *p){ // replace the 1st $ with PID
		  int x = getpid(); // get the pid
	          sprintf(dBuff, "%d", x); // expand it
		  strcat(buff,dBuff); // strcat it to buff
		  i++; // skips the next $
	       }
	       else{ // copy the next letter
		  int len = strlen(buff);
		  buff[len]=ptr[i]; // adds the next char to the buff string
	       }
	    }
	 }
	 strcpy(myst->args[index], buff);
	 index = index + 1;
	 myst->flags[0] = index; // store number of args to the flags
      }
   }while(ptr != NULL); // loops to strtok the rest of the input 
}

/***********************************
 * This function is the function for the built in status command, prints the exit status of last executed child
 * Input:
 * Output: text to stdout 
 **********************************/
void status()
{
   if (termSignal ==-55){ // if not a term signal
      printf("exit value %d\n",exitStatus); // print the exit status
      fflush(stdout);
   }
   else if (exitStatus == -55){ // if not an exit signal
      printf("terminated by signal %d\n", termSignal); // print the termination status
      fflush(stdout);
   }
}

/***********************************
 * This function process the foreground commands, it forks in foreground mode based on the provided input
 * Input: cmmdLine struct
 * Output: -  
 **********************************/
void fgComm(struct cmmdLine myst)
{
   int i, exitMethod;
   int inputFD, outputFD, dubResult = 0;
   pid_t spawnPid = -5;
   spawnPid = fork(); // fork
   if (spawnPid == -1){ // error forking
      perror("Error Forking\n");
      return;
   }
   else if (spawnPid == 0){ // for the child process

      SIGINT_action.sa_handler = SIG_DFL; // set up the SIGINT handler as default
      sigfillset(&SIGINT_action.sa_mask); // mask other signals
      SIGINT_action.sa_flags = 0;
      sigaction(SIGINT, &SIGINT_action, NULL); // call the sigaction 

      sigaction(SIGTSTP, &ignore_action, NULL); // set the SIGTSTP to be ignored

      if(myst.flags[1] == 1){ // if input flag is set
	 inputFD = open(myst.inputfile, O_RDONLY); // open the input file to read it with file descriptor inputFD
	 if (inputFD == -1) { perror("cannot open the input file"); exit(1); } // if it fails
	 dubResult = dup2(inputFD, 0); // redirect stdin to the input file using the FD
	 if (dubResult == -1) { perror("input dup2() failed"); exit(1); } // if it fails
	 fcntl(inputFD, F_SETFD, FD_CLOEXEC); // closes the file after exec() is done 
      }
      dubResult = 0;
      if (myst.flags[2] == 1){ // if output flag is set 
	 outputFD = open(myst.outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // open output file to write it with a file descriptor outputFD
	 if (outputFD == -1) { perror("cannot open the output file"); exit(1); } // if it fails
	 dubResult = dup2(outputFD, 1); // redirect stdout to the output file
	 if (dubResult == -1) { perror("output dup2() failed"); exit(1); } // if it fails
	 fcntl(outputFD, F_SETFD, FD_CLOEXEC); // closes file descriptor after exec is done
      }
      char *argv[513]; // declare an array of pointers
      for (i =0; i<myst.flags[0]; i++){ // assign it to the struct
	 *(argv+i) = myst.args[i];
      }
      *(argv+myst.flags[0])= NULL;
      if (execvp(*argv, argv) < 0) { // if exec fails
	 perror("Exec() failed");
	 exit(1);
      }
   }
   // for the parent to execute
   sigprocmask(SIG_BLOCK,&TSTPsig, NULL); // blocks SIGTSTP
   waitpid(spawnPid, &exitMethod, 0); // waits for child
   if (WIFEXITED(exitMethod)){ // regular termination
      termSignal = -55;
      exitStatus = WEXITSTATUS(exitMethod); // expand  it as int in exitStatus variable
   }
   else{ // signal termination 
      exitStatus = -55;
      termSignal = WTERMSIG(exitMethod); // expand it as int in termSignal variable
      status(); // call status
   }
   sigprocmask(SIG_UNBLOCK,&TSTPsig, NULL); // unblocks SIGTSTP
}

/***********************************
 * This function is the built in function for the cd command, it changes the directory based on the input 
 * Input: cmmdLine struct 
 * Output: -  
 **********************************/
void changeDir(struct cmmdLine myst)
{
   if (myst.flags[0] == 1) // if no argument was provided
      chdir(getenv("HOME")); // get the content of env. var HOME 
   else
      chdir(myst.args[1]); // if a path arg was provided
}

/***********************************
 * This function is the built in function for the exit command, it terminates the child(s) then exit
 * Input: runing PIDs list
 * Output: -  
 **********************************/
void myexit(int pids[])
{   
   int i, buff, exitMethod;
   int killState = -5;
   // This loop sends SIGTERM to all the running BG commands
   for(i = 1; i < pids[0]; i++){ // loops for a number of stored PIDs
      if(pids[i]>0){ // if it has not been terminated yet
	 do{
	    killState = kill(pids[i], SIGTERM); // kills all the running process
	 }while(killState !=0); // loops until it succeed
      }
   }
   // This loop waits for all the running commands to be killed
   for(i = 1; i < pids[0]; i++){ // loops for a number of stored PIDs
      if(pids[i]>0){ // if it has not been terminated yet
	 waitpid(pids[i], &exitMethod, 0); // check state
      }
   }
   exit(0); // exit the program 
}

/***********************************
 * This function process the background commands, it forks in background mode based on the provided input
 * Input: cmmdLine struct, list of running PIDs
 * Output: -  
 **********************************/
void bgComm(struct cmmdLine myst, int pids []){

   int i;
   int inputFD, outputFD, dubResult = 0;
   pid_t spawnPid = -5;
   spawnPid = fork();
   if (spawnPid == -1){ // error forking
      perror("Error Forking\n");
      return;
   }
   else if (spawnPid == 0){ // for the child process
      
      sigaction(SIGTSTP, &ignore_action, NULL); // set the SIGTSTP to be ignored

      if(myst.flags[1] == 1){ // if input flag is set
	 inputFD = open(myst.inputfile, O_RDONLY); // open the input file to read it with file descriptor inputFD
	 if (inputFD == -1) { perror("cannot open the input file"); exit(1); } // if it fails
	 dubResult = dup2(inputFD, 0); // redirect stdin to the input file using the FD
	 if (dubResult == -1) { perror("input dup2() failed"); exit(1); } // if it fails
	 fcntl(inputFD, F_SETFD, FD_CLOEXEC); // closes the file after exec() is done 
      }
      else { // no input file, redirect stdin to /dev/null
	 int inpDevNull=open("/dev/null", O_RDONLY); // have a file descriptor for /dev/null with stdin
	 if (inpDevNull == -1) { perror("cannot open /dev/null for input"); exit(1); } // if it fails
	 dubResult = dup2(inpDevNull, 0); // redirect stdin to /dev/null
	 if (dubResult == -1) { perror("input dup2() failed"); exit(1); } // if it fails
	 fcntl(inpDevNull, F_SETFD, FD_CLOEXEC); // closes the file after exec() is done 
      }
      dubResult = 0;
      if (myst.flags[2] == 1){ // if output flag is set 
	 outputFD = open(myst.outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // open output file to write it with a file descriptor
	 if (outputFD == -1) { perror("cannot open the output file"); exit(1); } // if it fails
	 dubResult = dup2(outputFD, 1); // redirect stdout to the output file
	 if (dubResult == -1) { perror("output dup2() failed"); exit(1); } // if it fails
	 fcntl(outputFD, F_SETFD, FD_CLOEXEC); // closes file descriptor after exec is done
      }
      else { // no output file was provided 
	 int outDevNull=open("/dev/null", O_WRONLY); // have a file descriptor for /dev/null with stdout
	 if (outDevNull == -1) { perror("cannot open /dev/null for input"); exit(1); } // if it fails
	 dubResult = dup2(outDevNull, 1); // redirect stdout to /dev/null
	 if (dubResult == -1) { perror("input dup2() failed"); exit(1); } // if it fails
	 fcntl(outDevNull, F_SETFD, FD_CLOEXEC); // closes the file after exec() is done 
      }
      char *argv[513]; // declare an array of pointers
      for (i =0; i<myst.flags[0]; i++){ // assign it to the struct
	 *(argv+i) = myst.args[i];
      }
      *(argv+myst.flags[0])= NULL;
      if (execvp(*argv, argv) < 0) { // if exec fails
	 perror("Exec() failed");
	 exit(1);
      }
   }
   // for the parent to execute
   pids[pids[0]] = spawnPid; // store the PID
   printf("background pid is %d\n", pids[pids[0]]); // print the PID
   fflush(stdout);
   pids[0] += 1; // increment # of PIDs
}

/***********************************
 * This function checks if the background running process has terminated or not using waitpd(), if yes report that
 * Input: list of running commands
 * Output: -  
 **********************************/
void bgCheck(int pids[]){

   int i, exitMethod, buff;

   for(i = 1; i < pids[0]; i++){ // loops for a number of stored PIDs
      if(pids[i]>0){ // if it has not been terminated yet
	 buff = waitpid(pids[i], &exitMethod, WNOHANG); // check state
	 if (buff>0){ // if it terminated
	    pids[i] = 0; // clear its position
	    printf("background pid %d is done: ", buff); // print message
	    fflush(stdout);
	    if (WIFEXITED(exitMethod)){ // regular termination
	       termSignal = -55;
	       exitStatus = WEXITSTATUS(exitMethod); // expand  it as int
	    }
	    else{ // signal termination 
	       exitStatus = -55;
	       termSignal = WTERMSIG(exitMethod); // expand it as int
	       // printf("terminated by signal %d\n", termSignal); // need to be handeled differently 
	       // fflush(stdout);
	    }
	    status(); // call status()
	 }
      }
   }
}

/**********************************
 * This is the main function of the program, and it derive the shell operation
 * ********************************/
int main() 
{
   char* usrInput;
   struct cmmdLine myst;
   int i;
   int myPIDs[1000]; // declared for PIDs, myPIDs[0] holds the number of running children, for efficiency might use malloc()

   sigemptyset(&TSTPsig); // empty the sig set
   sigaddset(&TSTPsig, SIGTSTP); // adds SIGTSTP to the set

   SIGINT_action.sa_handler = SIG_IGN; // set up the SIGINT handler as ignore
   sigaction(SIGINT, &SIGINT_action, NULL); // call the sigaction 

   SIGTSTP_action.sa_handler = catchSIGTSTP; // set up the SIGTSTP handler
   sigfillset(&SIGTSTP_action.sa_mask); // mask other SIGs
   SIGTSTP_action.sa_flags = 0; // set flag to zero
   sigaction(SIGTSTP, &SIGTSTP_action, NULL); // call the sigaction 

   ignore_action.sa_handler = SIG_IGN; // set it's handler to be ignored

   for(i=0;i<1000;i++) // this loop intialize the running commands to be zeros
      myPIDs[i]=0;
   myPIDs[0]=1;

   while(1){ // loops forever unless user enters exit
      clearCmmdLine(&myst); // clears the command line struct
      usrInput = getInput(myPIDs); // gets the user input
      processInput(&myst, usrInput); // process it to the struct 

      if(strcmp(myst.command, "exit") == 0){ // if the command is exit
	 myexit(myPIDs);
      }
      else if(strcmp(myst.command, "status") == 0){ // if the command is status
	 status();
      }
      else if(strcmp(myst.command, "cd") == 0){ // if the command is cd
	 changeDir(myst);
      }
      else {
	 if (myst.flags[3] ==0){ // if it's a forenground command
	    fgComm(myst); // call foreground command function
	 }
	 else { // it's a background command
	    bgComm(myst, myPIDs); // call background command function 
	 }
      }
      free(usrInput); // free the memory allocated by getline()
   } // end of the loop (loops again)

   /******************** THIS IS USED FOR TESTING ******************
     printf("command: %s\n", myst.command);
     printf("input: %s\n", myst.inputfile);
     printf("output: %s\n", myst.outputfile);
     printf("bg flag: %d\n", myst.flags[3]);
     printf("input flag: %d\n", myst.flags[1]);
     printf("output flag: %d\n", myst.flags[2]);
     printf("num of args: %d\n", myst.flags[0]);
     for(i=0;i<myst.flags[0]; i++)
     printf("arg %d is: %s\n", i+1, myst.args[i]);
    ***************************************************************/

   return 0; 
}

