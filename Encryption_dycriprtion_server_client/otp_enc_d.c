/***********************************
 * Author: Ibrahim Alarifi
 * Date: 08/11/2019
 * Discreption: This is a deamone server that receive a message and a key from a client, encrypt it, then send it back.
 * Output: sends a cypher key to the client
 * Input: message and a key from a client
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


/********************************
 * This function prints the error message then exits with 1
 * *****************************/
void error(const char *msg) { fprintf(stderr,msg); exit(1); } // Error function used for reporting issues

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
	 }
      }
   }
}

/***********************************
 * This function called by the child, takes the plain text and the key, encrypt the message and store it in cypTxt[]
 * Input: plainTxt array, keyTxt array, cypTxt array
 * Output: Store the encrypted message in cypTxt array
 **********************************/
void encrypt(char pT[], char kT[], char cypTxt[])
{
   int i, kBuff, pBuff, myres;

   for(i=0;i<strlen(pT); i++){ // this loop replace any 'space' with '[' in both plain text and key, to performe math later
      if(pT[i]==' ') // at plain text
	 pT[i]='[';
      if(kT[i]==' ') // at key txt
	 kT[i]='[';
   }
   for(i=0;i<strlen(pT); i++){ // this loop perform the encryption
      pBuff=(int)pT[i]; // convert the char to int at plaintxt[i]
      kBuff=(int)kT[i]; // convert the char to int at keytxt[i]
      pBuff-=65; // remove 65 offset
      kBuff-=65; // remove 65 offset
      myres=(pBuff+kBuff) % 27; // add them, then mod them by 27
      myres+=65; // add the 65 offset to the result
      cypTxt[i]=(char)myres; // convert it back to a char and store it
   }
   for(i=0;i<strlen(cypTxt);i++) // this loop replace back any '[' with 'space' in the cyptxt[]
      if(cypTxt[i]=='[')
	 cypTxt[i]=' ';
}

/***********************************
 * This function processes the child process by receiving the data then encrypt it then send it back
 * Input: the connection file descriptor 
 * Output: -  
 **********************************/
void processChild(int* estConFD)
{
   char plainBuff[100000];
   char keyBuff[100000];
   char cypTxt[100000];
   int sizee, totalCh;
   int charsRead, charsWritten;

   memset(plainBuff, '\0', 100000);
   memset(cypTxt, '\0', 100000);
   charsRead = recv(*estConFD, plainBuff, 3, 0); // Read the message, which is an identify message (supposed to be "enc" to be approved") 
   if (charsRead < 0) error("SERVER: ERROR reading from socket\n"); // error reading
   if(strcmp(plainBuff,"enc") !=0){ // if it's not the "enc" identify message
      // Send a refuse connection to the client
      charsRead = send(*estConFD, "noo", 3, 0); // Send no back
      if (charsRead < 0) error("SERVER: ERROR writing to socket\n"); // error writing
      close(*estConFD); // close the FD
      exit(2); // terminate child
   }

   else { // else if the client was identified
      // Send a Success message back to the client
      charsRead = send(*estConFD, "yes", 3, 0); // Send yes back
      if (charsRead < 0) error("SERVER: ERROR writing to socket\n"); // error sending
   }

   // Get the size message from the client
   memset(plainBuff, '\0', 100000);
   charsRead = recv(*estConFD, plainBuff, 10, 0); // Read the client's message from the socket
   if (charsRead < 0) error("SERVER: ERROR reading from socket\n"); // error reading
   sizee = atoi(plainBuff); // conver size to integer
//   printf("***%d***\n",sizee);
   // Get the plain text message from the client
   memset(plainBuff, '\0', 100000);
   totalCh=0;
   do{ // loops untill done reading
//      printf("S:L1\n");
      if((sizee-totalCh)<1000)
	 charsRead = recv(*estConFD, (plainBuff+totalCh), (sizee-totalCh), 0); // Read data from the socket
      else
	 charsRead = recv(*estConFD, (plainBuff+totalCh), 1000, 0); // Read the client's message from the socket in chunks of 1000 chars
      if (charsRead < 0) error("SERVER: ERROR reading from socket\n"); // error reading
      totalCh+=charsRead;
//      printf("S1:**%d**\n", charsRead);
   }while(totalCh<sizee);

   // Get the key text message from the client
   memset(keyBuff, '\0', 100000);
   totalCh=0;
   do{ // loops untill done reading
//      printf("S:L2\n");
      if((sizee-totalCh)<1000)
	 charsRead = recv(*estConFD, (keyBuff+totalCh), (sizee-totalCh), 0); // Read data from the socket
      else
	 charsRead = recv(*estConFD, (keyBuff+totalCh), 1000, 0); // Read the client's message from the socket in chunks of 1000 chars
      if (charsRead < 0) error("SERVER: ERROR reading from socket\n");
      totalCh+=charsRead; // increment offset
   }while(totalCh<sizee);

   encrypt(plainBuff, keyBuff, cypTxt); // call encrypt() to encrypt the message and store it to cypTxt array

   // Send a cypTxt to the client
   totalCh=0;
   do{ // loop untill done sending
//      printf("S:L3\n");
      if((strlen(cypTxt)-totalCh)<1000)
	 charsWritten = send(*estConFD, (cypTxt+totalCh), (strlen(cypTxt)-totalCh), 0); // Send in chuncks of 1000 chars
      else
	 charsWritten = send(*estConFD, (cypTxt+totalCh), 1000, 0); // Send in chuncks of 1000 chars
      if (charsWritten < 0) error("SERVER: ERROR writing to socket\n"); // if error
      totalCh+=charsWritten;
//      printf("S2:**%d**\n", charsWritten);
   }while(totalCh<strlen(cypTxt));
   close(*estConFD); // Close the existing socket which is connected to the client
}

/*************************************
 * This is the main function that drive the server to accept connections and for to process them.
 * **********************************/
int main(int argc, char *argv[])
{

   pid_t spawnPid = -5; // for forking
   int pids[2000]; // holds PIDs
   int i, listenSocketFD, establishedConnectionFD, portNumber, charsRead;
   socklen_t sizeOfClientInfo;
   struct sockaddr_in serverAddress, clientAddress;

   if (argc < 2) { error("SERVER: ERROR, wrong number of args\n"); } // Check correct # of args

   // Set up the address struct for this process (the server)
   memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

   for(i=0;i<2000;i++) // zero out the PIDs
      pids[i]=0;
   pids[0]=1; // set PIDs counter in pid[0]

   // Set up the socket
   listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (listenSocketFD < 0) error("SERVER: ERROR opening socket\n");

   // Enable the socket to begin listening
   if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
   {fprintf(stderr, "SERVER: ERROR on binding to %d\n", portNumber); exit(2);}
   listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
   while(1){ // infinite loop, stop by terminating
      bgCheck(pids); // call bgCheck to wait for childs to be terminated
      // Accept a connection, blocking if one is not available until one connects
      sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
      establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept in new FD
      if (establishedConnectionFD < 0) fprintf(stderr, "SERVER: ERROR on accept\n"); // if error
      else { // if not error fork
	 spawnPid = fork();
	 if (spawnPid == -1){ // error forking
	    fprintf(stderr, "SERVER: Error Forking\n");
	 }
	 else if (spawnPid == 0){ // for the child
	    processChild(&establishedConnectionFD); // process the encryption
	    exit(0); // exit
	 }
	 if(spawnPid !=-1){ // for the parent
	    pids[pids[0]] = spawnPid; // store the PID
	    pids[0] += 1; // increment # of PIDs
	 }
      }
   } // loops again

   close(listenSocketFD); // Close the listening socket
   return 0; 
}
