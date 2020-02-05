/***********************************
 * Author: Ibrahim Alarifi
 * Date: 07/22/2019
 * Discreption: Reads the rooms text files, and allow the user to nevigate between them, starting from the start room
 * 	when the user reaches end room, they win the game. It also allow the user to print the current time
 * Output: User interface
 * Restrictions: Needs to be compiled with the pthread librart (gcc -lpthread FILE_NAME.c)
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
#include <pthread.h> 


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; // declare the thread mutex

// declare the room struct, in which rooms will be read
struct room
{
   int roomId;
   char name[10];
   int numConnections;
   char connectedRooms[6][60];

};

// decalre the game struct, in which user will be nevigate
struct game
{
   int steps;
   struct room currentRoom;
};

/***********************************
 * This functions check what is the most recent file path and store it back in cstring
 * Input: file path cstring
 * Output: -
 **********************************/
void getRecentFile(char* filePath)
{
   struct stat dirStat; // used with the stat library
   time_t timeBuff=0;
   time_t prevBuff=0;
   struct dirent *dirContent; 
   
   DIR *dirDescriptor = opendir("."); // opendir() returns a pointer of DIR type to the current directory.  

   memset(filePath, '\0', sizeof(filePath)); // initilze the filePath string with null chars
   // loops tell the end of the directory content
   while ((dirContent = readdir(dirDescriptor)) != NULL){ 
      stat(dirContent->d_name, &dirStat); // retrieve information about the file pointed to by dirContent->d_name to the stat struct
      if ((dirContent->d_name[0] == 'a') && ((dirStat.st_mode & S_IFMT) == S_IFDIR)){ // if it starts with "a" (my last name) and is a directory 
	 timeBuff = dirStat.st_mtime; // update the time buffer
	 if (timeBuff>prevBuff){ // compare time buffer with the previous one ( in in the loop iteration
	    strcpy(filePath, dirContent->d_name); // if true, update the file path and the previous time buffer to future comparsion
	    prevBuff=timeBuff;
	    timeBuff=0;
	 } // keeps looping tell the end of the directory
      }
   }
   closedir(dirDescriptor);  // close the directory
}

/***********************************
 * This functions opens a file specified and check the number of connections, then returns it
 * Input: file path
 * Output: Returns the number of connections in the room (int)
 **********************************/
int getNumConnections(char * path)
{
   int numConnection=0;
   char mych;  // store a character read from file 
   FILE* myFile; // file pointer

   // open the file 
   myFile = fopen(path, "r"); // read only 
   while ((mych=getc(myFile)) != EOF){ // loops to the end of the file, reads each charecter to mych at each iteration
      if (mych=='\n') // if it incounters a new line
	 numConnection +=1; // store number of lines in that file
   }
   fclose(myFile); // close the file

   return numConnection - 2; // return number of line - 2 ( Room name and Room type lines)
}

/***********************************
 * This functions is a pthread function, it gets the current time and store it in a text file
 * Input: - 
 * Output: writes the current time into a text file 
 **********************************/
void *writeTime (void* arg) 
{
   pthread_mutex_lock(&mutex1); // lock the mutex
   char currTime[200];
   time_t t;
   struct tm *tmp;
   int file_descriptor, i, x=0;
   ssize_t nwritten;
   char buff[80];


   FILE *fptr = fopen("./currentTime.txt", "w"); // open the file path with writing premission

   memset(currTime, '\0', sizeof(currTime)); // clears the currTime string with null char
   memset(buff, '\0', sizeof(buff)); // clears buff string with null char
   t = time(NULL); // get current time to t
   tmp = localtime(&t); // put it to the tmp struct to be used with strftime()

   strftime(buff, sizeof(buff), "%I", tmp); // gets the hour
   strcat(currTime,buff); // strcat it to currTime
   strcat(currTime, ":"); // strcat a : to seprate mins
   memset(buff, '\0', sizeof(buff)); // clears buff with null char

   strftime(buff, sizeof(buff), "%M", tmp); // gets minutes
   strcat(currTime,buff); // strcat it to currTime
   memset(buff, '\0', sizeof(buff)); 

   strftime(buff, sizeof(buff), "%P", tmp); // gets am/pm
   strcat(currTime,buff); // strcat it to currTime
   strcat(currTime,", ");
   memset(buff, '\0', sizeof(buff));

   strftime(buff, sizeof(buff), "%A", tmp); // gets day 
   strcat(currTime,buff); // strcat it to currTime
   strcat(currTime,", ");
   memset(buff, '\0', sizeof(buff)); 

   strftime(buff, sizeof(buff), "%B", tmp); // gets month name
   strcat(currTime,buff); // strcat it to currTime
   strcat(currTime," ");
   memset(buff, '\0', sizeof(buff)); 

   strftime(buff, sizeof(buff), "%d", tmp); // gets date number
   strcat(currTime,buff); // strcat it to currTime
   strcat(currTime,", ");
   memset(buff, '\0', sizeof(buff)); 

   strftime(buff, sizeof(buff), "%Y", tmp); // gets year
   strcat(currTime,buff); // strcat it to currTime

   fprintf(fptr,"%s\n", currTime);  // write currTime to the text file

   fclose(fptr); // close the file
   pthread_mutex_unlock(&mutex1); // unlock the mutex
   return NULL; // return null

}

/***********************************
 * This functions reads the txt file in path, then store it to a room struct then returns it 
 * Input: room ID and a file path
 * Output: Returns a room struct that is based on info contained in the txt file
 **********************************/
struct room readFilesToStructs(char* path, int theID)
{
   int fileDes, newPosition, i=0;
   struct room outputRoom; // declare a room struct 
   ssize_t nread;

   for(i=0; i<6; i++){ // this loop clears the struct connections with null char
      memset(outputRoom.connectedRooms[i], '\0', sizeof(outputRoom.connectedRooms[i]));
   }

   outputRoom.roomId = theID; // initlize the room ID with the input theID
   outputRoom.numConnections=getNumConnections(path); // call getNumConnections to store it in the struct
   fileDes=open(path, O_RDONLY); // opens the file with read only premission

   newPosition = lseek(fileDes,11 , SEEK_SET); // skips ROOM NAME: to get to the actual name
   nread = read(fileDes, outputRoom.name, 8); // reads the name to the struct
   outputRoom.name[7]='\0'; // clears the new line char in the name 
   for (i=0; i < outputRoom.numConnections; i++){ // reads the connections in the loop based on what getNumConncetion returned
      newPosition = lseek(fileDes,14, SEEK_CUR); // skips CONNECTION n: to get to the actual name
      nread = read(fileDes, outputRoom.connectedRooms[i], 8); // store it to the struct
      outputRoom.connectedRooms[i][7] = '\0'; // remove the new line char
   }
   close(fileDes); // close the file 

   return outputRoom; // returns the struct
}

/***********************************
 * This functions assign a game struct with the provided input and return it
 * Input: room struct, number of steps taken
 * Output: Returns a game struct with the current room location and the number of steps taken 
 **********************************/
struct game assignGame(struct room curRoom, int stps)
{
   struct game myG;

   myG.steps = stps;
   myG.currentRoom = curRoom;

   return myG;
}

/***********************************
 * This functions interact with the user to get the input from them, then returns it
 * Input: game struct that is keeping track of the user
 * Output: Returns a string input afet validating its correctness
 **********************************/
char* getInput(struct game* myG)
{
   char* readBuff=NULL;
   size_t buffSize=0;
   int letters;
   int i, checkCount=0;
   int printCount=0;

   while(1){ // loops until it gets a proper input 
      if (printCount==0){ // print the current state of the game
      printf("CURRENT LOCATION: %s\n", myG->currentRoom.name); // current location
      printf("POSSIBLE CONNECTIONS: ");
      
      for(i=0;i<myG->currentRoom.numConnections - 1;i++) // current connections
	 printf("%s, ", myG->currentRoom.connectedRooms[i]);
      printf("%s.\n", myG->currentRoom.connectedRooms[myG->currentRoom.numConnections - 1]);
      }
      printCount=0;
      printf("WHERE TO? >"); // ask the user to input data
      letters = getline(&readBuff, &buffSize, stdin); // reads input from stdin to readBuff
      printf("\n");
      readBuff[letters - 1] = '\0'; // remove the new line char from the input 
      for(i=0;i<myG->currentRoom.numConnections;i++) // checks if the input is valid (one of the connections)
	 if(strcmp(readBuff, myG->currentRoom.connectedRooms[i])==0) // if yes
	    checkCount +=1;
      if(checkCount != 0) // if it's one of the connections, return that input
	 return readBuff;
      else if(strcmp(readBuff, "time") ==0){ // if not check if it's time 
	 int rc1, i;
	 pthread_t thread1; // declare a thread
	 rc1=pthread_create(&thread1, NULL, &writeTime, NULL); // create it at writeTime() 
	 pthread_mutex_unlock(&mutex1); // unlock the mutex
	 pthread_join(thread1, NULL);  // make sure thread1 done
	 pthread_mutex_lock(&mutex1); // lock the mutex back
	 FILE *stream;
	 char *line = NULL;
	 size_t len = 0;
	 ssize_t nread; 
	 stream = fopen("./currentTime.txt", "r"); // open the time txt file 
	 nread = getline(&line, &len, stream); // read the time to line 
	 printf("%s\n", line); // print it
	 free(line); // free line 
	 fclose(stream); // close the txt file 
	 pthread_cancel(thread1); // cancel the thread
	 printCount=1;
      }
      else // invalid input
      printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      
      checkCount=0;
   } // loops again 
}

/**********************************
 * This is the main function of the program, and it derive the game flow
 * ********************************/
int main() 
{
   char filePath[30];
   char tempDir[30];
   struct room rooms[7]; // array of rooms
   int i=0;
   struct game myGame;
   int stpCount=0;
   char* readBuff=NULL;
   char **pathKeeper; // keep track of the path (names of the rooms visited)
   char **tempp;
   int oldSize;
   int pathElems = 10;

   pthread_mutex_lock(&mutex1); // initially lock the mutex

   pathKeeper = malloc(pathElems * sizeof(char*)); // create 2d dynamic memory of pathKeeper
   for (i = 0; i < pathElems; i++)
      pathKeeper[i] = malloc((8) * sizeof(char)); 



   getRecentFile(filePath); // call getRecentFile to get the recent filepath directory
   strcpy(tempDir, filePath); // copy it to tempDir
   rooms[0]=readFilesToStructs(strcat(tempDir,"/startRoom"), 0); // reads the startRoom.txt to the 1st room in the room array
   strcpy(tempDir, filePath); // reset tempDir
   rooms[1]=readFilesToStructs(strcat(tempDir,"/midRoom1"), 1); // middle room #1
   strcpy(tempDir, filePath);
   rooms[2]=readFilesToStructs(strcat(tempDir,"/midRoom2"), 2); // middle room #2
   strcpy(tempDir, filePath);
   rooms[3]=readFilesToStructs(strcat(tempDir,"/midRoom3"), 3); // middle room #3
   strcpy(tempDir, filePath);
   rooms[4]=readFilesToStructs(strcat(tempDir,"/midRoom4"), 4); // middle room #4
   strcpy(tempDir, filePath);
   rooms[5]=readFilesToStructs(strcat(tempDir,"/midRoom5"), 5); // middle room #5
   strcpy(tempDir, filePath);
   rooms[6]=readFilesToStructs(strcat(tempDir,"/endRoom"), 6); // end room

   myGame=assignGame(rooms[stpCount],stpCount); // assign the current location to the start room

   do{ // loops untill the player reaches the end room
      readBuff=getInput(&myGame); // call getInput to get the connection name that the user want to move to
      for(i=0; i < 7; i++){
	 if (strcmp(readBuff, rooms[i].name) == 0){ // get the room that the user want to move to from the room array
	    myGame=assignGame(rooms[i],stpCount+1); // assign the current location to that room and update the number of steps
	    strcpy(pathKeeper[stpCount], readBuff); // store it to the path keeper
	    stpCount +=1;
	    if(stpCount == (pathElems-1)){ // checks if we need to expand the memory of pathKeeper
	       oldSize=pathElems;
	       pathElems=pathElems*2; // doubles the size
	       pathKeeper = (char **) realloc(pathKeeper, pathElems * sizeof(char**)); // reallocate it 
	       for(i=oldSize; i<pathElems; ++i) // make new memory for the new elements
		  *(pathKeeper+i) = (char *)malloc(sizeof(char)*8);
	    }
	 }

      }
      free(readBuff); // frees the readbuff
      readBuff=NULL;
   }while(myGame.currentRoom.roomId != rooms[6].roomId); // check the state of the current location, if not the end room

   printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n"); // winning message
   printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", myGame.steps); // number of steps
   for(i=0; i < myGame.steps; i++) // path taken 
      printf("%s\n", pathKeeper[i]);

   for(i = 0; i < pathElems; i++) // free path keeper
      free(pathKeeper[i]);
   free(pathKeeper);

   pthread_mutex_destroy(&mutex1); // destroy the mutex 
   return 0; 
}

