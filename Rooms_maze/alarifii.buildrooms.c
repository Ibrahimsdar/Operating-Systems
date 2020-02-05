/***********************************
 * Author: Ibrahim Alarifi
 * Date: 07/22/2019
 * Discreption: intilize 7 rooms and connect them together randomly, then store them indivisually in text files
 * Output: 7 text files, one for each room
 **********************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/stat.h>
#include <unistd.h>

#define NUMBER_OF_ROOMS 10
#define MAX_STRING_SIZE 80


// declare a struct room
struct room
{   
   int roomId;
   char name[10];
   int numConnections;
   struct room* connectedRoom[6];
};


/***********************************
 * This functions initilze a rom struct with the provided input and return it
 * Input: room name and room ID
 * Output: Returns a room struct initilzed with the name and the ID
 **********************************/
struct room roomInit(char* name, int theID )
{
   struct room outputRoom;
   outputRoom.roomId = theID;
   outputRoom.numConnections=0;
   strcpy(outputRoom.name, name);

   return outputRoom;
}

/********** Printing Function For Testing ***********\
void PrintRoomOutboundConnections(struct room* input)
{
   printf("The rooms connected to (%s/%d) are:\n",
	 input->name, input->roomId);

   int i;
   for (i = 0; i < input->numConnections; i++)
      printf("  (%s/%d)\n", input->connectedRoom[i]->name,
	    input->connectedRoom[i]->roomId);
   return;
}
\*****************************************************/

/***********************************
 * This functions generate a random number between 0-9 and make sure it wasn't chosen before
 * Input: int array
 * Output: Returns a random number between 0-9, and store in int arr[] to avoid chosing it again
 **********************************/
int randomNum(int arr[])
{
   int myRand, i, repCount=0;
   while (1){
      myRand=(rand()%10); // generate a number between 0-9
      for(i=0;i<7;i++){
	 if(arr[i] == myRand){ // checks that it's not contained in arr, since it gets stored in arr[] in the main function
	    repCount++;
	 }
      }
      if (repCount==0){ // if it's not in arr[] return it, if not loop again 
	 return myRand;
      }
      repCount=0;
   }
}

/***********************************
 * This functions checks if all rooms have at least 3 outbound connections
 * Input: 7 rooms structs
 * Output: Returns 1 if all rooms have 3 to 6 outbound connections, 0 otherwise
 **********************************/
int fullConnections(struct room* s, struct room* m1, struct room* m2, struct room* m3, struct room* m4, struct room* m5, struct room* e)  
{
   if ((s->numConnections >= 3) && (m1->numConnections >= 3) && (m2->numConnections >= 3) && (m3->numConnections >= 3) && (m4->numConnections >= 3) && (m5->numConnections >= 3) && (e->numConnections >= 3))
   {
      return 1;
   }
   else
      return 0;
}

/***********************************
 * This functions takes 7 structs and choose one of them randomly to return it
 * Input: 7 rooms structs
 * Output: Returns one room out of the seven randomly
 **********************************/
struct room* pickRoom(struct room* s, struct room* m1, struct room* m2, struct room* m3, struct room* m4, struct room* m5, struct room* e)
{
   int myRand=0;
   myRand=(rand()%7); // choose number between 0 and 6 randomly

   if(myRand == 0)
      return s; // return start room
   else if (myRand==1)
      return m1; // return mid 1 
   else if (myRand==2)
      return m2;
   else if (myRand==3)
      return m3;
   else if (myRand==4)
      return m4;
   else if (myRand==5)
      return m5;
   else 
      return e; // return end room if 6
}

/***********************************
 * This functions connects rooms A and B together
 * Input: two room structs, A and B
 * Output: - 
 **********************************/
void makeConnection(struct room* A, struct room* B) 
{
   A->connectedRoom[A->numConnections] = B; // adds B to A's connections list
   A->numConnections += 1; // increment A's number of connections
}

/***********************************
 * This functions take two rooms' IDs and check if they're the same
 * Input: Rooms A and B IDs (int)
 * Output: Returns 1 if Rooms A and B are the same Room, 0 otherwise
 **********************************/
int sameRoom(int A, int B) 
{

   if (A==B)
      return 1;
   else
      return 0;

}

/***********************************
 * This functions take number of connections in a given room to check if we can add more (<6)
 * Input: number of connections (int)
 * Output: Returns 1 if a connection can be added from Room A (< 6 outbound connections), 0 otherwise
 **********************************/
int checkNumConnections(int numCon) 
{
   if (numCon < 6)
      return 1;
   else
      return 0;
}

/***********************************
 * This functions take two structs and validate if connection is already there or no
 * Input: two structs, A and B
 * Output: Returns 1 if a connection from Room A to Room B already exists, 0 otherwise
 **********************************/
int alreadyConnected(struct room* A, struct room* B)
{
   int result=0;
   int i;

   // loops to check if A exsit in B's connections
   for(i=0; i < B->numConnections; i++)
      if (A->roomId == B->connectedRoom[i]->roomId)
	 result = 1;

   return result;
}

/***********************************
 * This function write the myst struct to a txt file contained in newFilePath
 * Input: the room struct, the file path, the room type
 * Output: -
 **********************************/
void writeFiles (struct room* myst, char* newFilePath, char* roomType) 
{
   int file_descriptor, i, x=0;
   ssize_t nwritten;
   char buff[80];
   
   memset(buff, '\0', sizeof(buff)); // initilize the buffer with null char 
   file_descriptor = open(newFilePath, O_WRONLY | O_APPEND | O_CREAT, 0600); // creats a file to write in
   
   nwritten = write(file_descriptor, "ROOM NAME: ", strlen("ROOM NAME: ") * sizeof(char)); // write in the file
   nwritten = write(file_descriptor, myst->name, strlen(myst->name) * sizeof(char)); // write the room name
   // this loop writes the connections to the room
   for(i=0; i < myst->numConnections; i++){
      sprintf(buff, "%d: ", i+1);
      nwritten = write(file_descriptor, "\nCONNECTION ", strlen("\nCONNECTION ") * sizeof(char));
      nwritten = write(file_descriptor, buff, strlen(buff) * sizeof(char));
      nwritten = write(file_descriptor, myst->connectedRoom[i]->name, strlen(myst->connectedRoom[i]->name) * sizeof(char));
   }
   nwritten = write(file_descriptor, "\nROOM TYPE: ", strlen("\nROOM TYPE: ") * sizeof(char));
   nwritten = write(file_descriptor, roomType, strlen(roomType) * sizeof(char)); // write the room type to the file
   nwritten = write(file_descriptor, "\n", strlen("\n") * sizeof(char));
   close(file_descriptor); // closes the file
}

/**********************************
 * This is the main function of the program, and it derive the rooms creation process
 * ********************************/
int main()
{  
   char roomArr[NUMBER_OF_ROOMS][MAX_STRING_SIZE] = 	// an array of ten rooms
   {  "Beijing",
      "Yerevan",
      "Nicosia",
      "Jakarta",
      "Vilnius",
      "Colombo",
      "Khobbar",
      "Nairobi",
      "Caracas",
      "Rangoon"
   };
   int pickedRooms=0;
   int roomKeeper[7];	// used to keeps track of the rooms picked
   int i=0;
   int buff=0;
   struct room startRoom;
   struct room mRoom1;
   struct room mRoom2;
   struct room mRoom3;
   struct room mRoom4;
   struct room mRoom5;
   struct room endRoom;
   struct room* roomA;
   struct room* roomB;
   srand(time(NULL));
   char roomDir[50] = "alarifii.rooms."; // directory in which txt files will be stored in 

   for(i=0; i < 7; i++)	// initilize the array to random value
      roomKeeper[i]=344;

   // this loop assign 7 rooms out of the 10 rooms in the array
   do{
      if (pickedRooms==0) { // start room
	 buff=randomNum(roomKeeper); // call randomNum to generate a room # that have not been picked
	 startRoom=roomInit(roomArr[buff], pickedRooms);  // intilize the struct with that room name
	 roomKeeper[pickedRooms]=buff;	// store the room number in the room keeper array
	 pickedRooms++;
      }
      else if (pickedRooms==1) {  // middle room 1 has the same process as start room
	 buff=randomNum(roomKeeper);
	 mRoom1=roomInit(roomArr[buff], pickedRooms);
	 roomKeeper[pickedRooms]=buff;
	 pickedRooms++;
      }
      else if (pickedRooms==2) { // middle room 2 has the same process as start room
	 buff=randomNum(roomKeeper);
	 mRoom2=roomInit(roomArr[buff], pickedRooms);
	 roomKeeper[pickedRooms]=buff;
	 pickedRooms++;
      }
      else if (pickedRooms==3) { // middle room 3 has the same process as start room
	 buff=randomNum(roomKeeper);
	 mRoom3=roomInit(roomArr[buff], pickedRooms);
	 roomKeeper[pickedRooms]=buff;
	 pickedRooms++;
      }
      else if (pickedRooms==4) { // middle room 4 has the same process as start room
	 buff=randomNum(roomKeeper);
	 mRoom4=roomInit(roomArr[buff], pickedRooms);
	 roomKeeper[pickedRooms]=buff;
	 pickedRooms++;
      }
      else if (pickedRooms==5) { // middle room 5 has the same process as start room
	 buff=randomNum(roomKeeper);
	 mRoom5=roomInit(roomArr[buff], pickedRooms);
	 roomKeeper[pickedRooms]=buff;
	 pickedRooms++;
      }
      else if (pickedRooms==6) { // end room has the same process as start room
	 buff=randomNum(roomKeeper);
	 endRoom=roomInit(roomArr[buff], pickedRooms);
	 roomKeeper[pickedRooms]=buff;
	 pickedRooms++;
      }
   }while(pickedRooms<7);

   // while all the rooms don't have at least 3 connections
   while (fullConnections(&startRoom, &mRoom1, &mRoom2, &mRoom3, &mRoom4, &mRoom5, &endRoom) == 0)
   {
      // loops for ever until it break
      while(1)
      {
	 // randomly assign a room to roomA
	 roomA = pickRoom(&startRoom, &mRoom1, &mRoom2, &mRoom3, &mRoom4, &mRoom5, &endRoom);
	 if (checkNumConnections(roomA->numConnections) == 1) // if the room has less than 6 connections (valid to connect)
	    break;
      }
      // loop to assign a room to roomB that is valid to connect to, will break of the loop when it's valid
      do
      {
	 roomB = pickRoom(&startRoom, &mRoom1, &mRoom2, &mRoom3, &mRoom4, &mRoom5, &endRoom);
      }while(checkNumConnections(roomB->numConnections) == 0 || sameRoom(roomA->roomId, roomB->roomId) == 1 || alreadyConnected(roomA, roomB) == 1);
      makeConnection(roomA, roomB);  // connect A to B
      makeConnection(roomB, roomA);  // connect B to A
   }
   
   // this loops gets the process ID and strcat it to the folder name stored in roomDir
   do
   {
      char temp[20];
      int x = getpid();
      memset(temp, '\0', sizeof(temp));
      sprintf(temp, "%d", x);
      strcat(roomDir,temp);
      buff=mkdir(roomDir, 0700);
   }while(buff == -1);

   char tempDir[50];

   strcpy(tempDir, roomDir); // copy the path to tempDire
   writeFiles(&startRoom, strcat(tempDir,"/startRoom"), "START_ROOM"); // write the struct to a text file by calling writeFiles()
   strcpy(tempDir, roomDir); // copy the path to tempDir
   writeFiles(&mRoom1, strcat(tempDir,"/midRoom1"), "MID_ROOM"); // write the struct to a text file by calling writeFiles()
   strcpy(tempDir, roomDir); // copy the path to tempDir
   writeFiles(&mRoom2, strcat(tempDir,"/midRoom2"), "MID_ROOM"); // write the struct to a text file by calling writeFiles()
   strcpy(tempDir, roomDir); // copy the path to tempDir
   writeFiles(&mRoom3, strcat(tempDir,"/midRoom3"), "MID_ROOM"); // write the struct to a text file by calling writeFiles()
   strcpy(tempDir, roomDir); // copy the path to tempDir
   writeFiles(&mRoom4, strcat(tempDir,"/midRoom4"), "MID_ROOM"); // write the struct to a text file by calling writeFiles()
   strcpy(tempDir, roomDir); // copy the path to tempDir
   writeFiles(&mRoom5, strcat(tempDir,"/midRoom5"), "MID_ROOM"); // write the struct to a text file by calling writeFiles()
   strcpy(tempDir, roomDir); // copy the path to tempDir
   writeFiles(&endRoom, strcat(tempDir,"/endRoom"), "END_ROOM"); // write the struct to a text file by calling writeFiles()
  
   /* print statements (used for testing)
   PrintRoomOutboundConnections(&startRoom);
   PrintRoomOutboundConnections(&mRoom1);
   PrintRoomOutboundConnections(&mRoom2);
   PrintRoomOutboundConnections(&mRoom3);
   PrintRoomOutboundConnections(&mRoom4);
   PrintRoomOutboundConnections(&mRoom5);
   PrintRoomOutboundConnections(&endRoom); 
   */
   return 0;
}
