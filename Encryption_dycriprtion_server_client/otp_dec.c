/***********************************
 * Author: Ibrahim Alarifi
 * Date: 08/11/2019
 * Discreption: This client code communicate with a server providing it cypher txt and key, then
 * 		  it recieve the plain txt and print it to stdout
 * Output: print plain txt to stdout
 * Input: cypher message and a key txt file from the user
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

/***********************************
 * This function prints an error to the user and exits with 1
 **********************************/
void error(const char *msg) { fprintf(stderr,msg); exit(1); } // Error function used for reporting issues

/***********************************
 * This function validate the provided text fileis, if passed, store it in seprate arrays
 * Input: two file pats, two array pointers
 * Output: if passed, store the proper data in the array pointers
 **********************************/
void checkFiles(char * cFile, char * kFile, char** readC, char** readK)
{
   FILE *cFD;
   FILE *kFD;
   char c;
   int nCypCh=0;
   int nKeyCh = 0;
   int i, badChFlag = 0;
   size_t buffSize=0;
   char * readBuff=0;

   cFD = fopen(cFile,"r"); // open the cyper text file
   if (cFD == NULL) { error("cannot open the cyper text file\n"); } // if it fails
   nCypCh = getline(&readBuff, &buffSize, cFD); // reads input from stdin to readBuff
   readBuff[nCypCh - 1] = '\0'; // remove the new line char from the input
   --nCypCh;
   for(i=0;i<nCypCh;i++){ // loops to check on all chars in the 1st file
      c = readBuff[i];
      if((c < 'A' && c!= ' ')||(c > 'Z')) // if the char is not in [A-Z] or not " "
	 badChFlag = 1; // set bad char flag
   }
   *readC = readBuff; // assign the pointer to the readBuff

   readBuff=NULL;
   buffSize=0;
   kFD = fopen(kFile, "r"); // open the key file
   if (kFD == NULL) { error("cannot open the key text file\n"); } // if it fails
   nKeyCh = getline(&readBuff, &buffSize, kFD); // reads input from stdin to readBuff
   readBuff[nKeyCh - 1] = '\0'; // remove the new line char from the input
   nKeyCh -= 1;
   for(i=0;i<nKeyCh;i++){ // loops to check on all chars in the 2nd file
      c = readBuff[i];
      if((c < 'A' && c!= ' ')||(c > 'Z')) // if bad char
	 badChFlag += 2; // set bad char flag
   }
   *readK = readBuff; // assign the pointer to the readBuff
   fclose(cFD); fclose(kFD); // close FDs
   if(nKeyCh<nCypCh) {free(*readK); free(*readC);fprintf(stderr,"CLIENT: ERROR, key '%s' is too short\n",kFile); exit(1);} // if the key is shorter
   if(badChFlag == 1) {free(*readK); free(*readC);fprintf(stderr,"CLIENT: ERROR, input '%s' contains bad characters\n",cFile);exit(1);} // if the bad chat flag is set
   if(badChFlag == 2) {free(*readK); free(*readC);fprintf(stderr,"CLIENT: ERROR, input '%s' contains bad characters\n",kFile);exit(1);} // if the bad chat flag is set
   if(badChFlag == 3) {free(*readK); free(*readC);fprintf(stderr,"CLIENT: ERROR, input '%s and %s' contains bad characters\n",cFile, kFile);exit(1);} // if the bad chat flag is set
}

/********************************
 * This is the main function that drives the client code, it reads input, then process it and communicate it with the server
 * *****************************/
int main(int argc, char *argv[])
{

   char* cFile;
   char* kFile;
   char* cText=NULL;
   char* kText=NULL;
   char buffer[100000];
   int socketFD, portNumber, charsWritten, charsRead, totalCh;
   struct sockaddr_in serverAddress;
   struct hostent* serverHostInfo;

   if (argc < 4) { error("ERROR, wrong number of args\n"); } // Check usage & args
   cFile = argv[1];
   kFile = argv[2];

   checkFiles(cFile, kFile, &cText, &kText); // checks if the files are satisfy the encryption conditions

   // Set up the server address struct
   memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
   if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(1); }
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

   // Set up the socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (socketFD < 0) error("CLIENT: ERROR opening socket\n");

   // Connect to server
   if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
   {fprintf(stderr, "CLIENT: ERROR, could not contact otp_dec_d on port %d\n", portNumber); exit(2);}
   // Send message to server
   charsWritten = send(socketFD, "dyc", 3, 0); // Write to the server
   if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");

   // Get return message from server
   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
   charsRead = recv(socketFD, buffer, 3, 0); // Read data from the socket, leaving \0 at end
   if (charsRead < 0) error("CLIENT: ERROR reading from socket\n");
   if (strcmp(buffer,"yes")!=0) {fprintf(stderr, "CLIENT: ERROR, could not contact otp_dec_d on port %d since it's assosiated with otp_enc_d\n", portNumber); exit(2);} // invalid communication

   // Send message size to server
   char sizeBuff[10];
   memset(sizeBuff, '\0', sizeof(sizeBuff)); // Clear out the buffer again for reuse
   snprintf(sizeBuff, 10, "%d", strlen(cText)); // convert it from int to c-string
   charsWritten = send(socketFD, sizeBuff, sizeof(sizeBuff), 0); // Write the size to the server
   if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");

   // Send plain text to server
   totalCh=0;
   do{ // loop untill done sending
      //      printf("C:L1\n");
      if((strlen(cText)-totalCh) < 1000)
	 charsWritten = send(socketFD, (cText+totalCh), (strlen(cText)-totalCh), 0); // Write to the server in chuncks of 1000
      else
	 charsWritten = send(socketFD, (cText+totalCh), 1000, 0); // Write to the server in chuncks of 1000
      if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");
      totalCh+=charsWritten; // increment offset 
      //      printf("C1:**%d**\n", charsWritten);
   }while(totalCh < strlen(cText));

   // Send key text to server
   totalCh=0;
   do{ // loop untill all chars are sent
      //      printf("C:L2\n");
      if((strlen(cText)-totalCh) < 1000)
	 charsWritten = send(socketFD, (kText+totalCh), (strlen(cText)-totalCh), 0); // Write to the server in chuncks of 1000
      else
	 charsWritten = send(socketFD, (kText+totalCh), 1000, 0); // Write to the server in chuncks of 1000
      if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");
      totalCh+=charsWritten; // increment offset
   }while(totalCh < strlen(cText));

   // Get decrypted text from server
   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
   totalCh=0;
   do{ // loops untill all chars are read
      //     printf("C:L3\n");
      if((strlen(cText)-totalCh) < 1000)
	 charsRead = recv(socketFD, (buffer + totalCh), (strlen(cText)-totalCh), 0); // Read data from the socket in chuncks of 1000 chars
      else
	 charsRead = recv(socketFD, (buffer + totalCh), 1000, 0); // Read data from the socket in chuncks of 1000 chars
      if (charsRead < 0) error("CLIENT: ERROR reading from socket\n");
      totalCh+=charsRead; // inc. offset
      //      printf("C2:**%d**\n", charsRead);
   }while(totalCh < strlen(cText));
   printf("%s\n", buffer); // print the message to stdout

   close(socketFD); // Close the socket
   free(cText); // free memory
   free(kText); // free memory 
   return 0;
}
