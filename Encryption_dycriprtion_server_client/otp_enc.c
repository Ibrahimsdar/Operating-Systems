/***********************************
 * Author: Ibrahim Alarifi
 * Date: 08/11/2019
 * Discreption: This client code communicate with a server providing it message and key, then
 * 		  it recieve the cypher txt and print it to stdout
 * Output: print cypher txt to stdout
 * Input: message and a key txt file from the user
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

/********************************
 * This function prints the error message then exits with 1
 * *****************************/
void error(const char *msg) {fprintf(stderr,msg); exit(1); } // Error function used for reporting issues

/***********************************
 * This function validate the provided text fileis, if passed, store it in seprate arrays
 * Input: two file pats, two array pointers
 * Output: if passed, store the proper data in the array pointers
 **********************************/
void checkFiles(char * pFile, char * kFile, char** readP, char** readK)
{
   FILE *pFD;
   FILE *kFD;
   char c;
   int nPlainCh=0;
   int nKeyCh = 0;
   int i, badChFlag = 0;
   size_t buffSize=0;
   char * readBuff=NULL;

   pFD = fopen(pFile,"r"); // open the plaintext file
   if (pFD == NULL) { error("CLIENT: cannot open the input file\n"); } // if it fails
   nPlainCh = getline(&readBuff, &buffSize, pFD); // reads input from stdin to readBuff
   readBuff[nPlainCh - 1] = '\0'; // remove the new line char from the input
   --nPlainCh; // decrement # of chars

   for(i=0;i<nPlainCh;i++){ // this loop checks on all the chars in the plaintxt file
      c = readBuff[i];
      if((c < 'A' && c!= ' ')||(c > 'Z')) // if the char is not in [A-Z] or " "
	 badChFlag = 1; // set fag
   }
   *readP = readBuff; // assign the plaintxt pointer to the data that has been read

   readBuff=NULL;
   buffSize=0;
   kFD = fopen(kFile, "r"); // open the key file
   if (kFD == NULL) { error("CLIENT: cannot open the input file\n"); } // if it fails
   nKeyCh = getline(&readBuff, &buffSize, kFD); // reads input from stdin to readBuff
   readBuff[nKeyCh - 1] = '\0'; // remove the new line char from the input
   nKeyCh -= 1; // dec # of chars
   for(i=0;i<nKeyCh;i++){ // loops to check on all chars
      c = readBuff[i];
      if((c < 'A' && c!= ' ')||(c > 'Z')) // if it's a bad char
	 badChFlag +=2; // set the flag
   }
   *readK = readBuff; // assign the key array to the readBuff
   fclose(pFD); fclose(kFD); // close FDs
   if(nKeyCh<nPlainCh) {free(*readK); free(*readP);fprintf(stderr,"CLIENT: ERROR, key '%s' is too short\n",kFile);exit(1);} //if the key is less than the message
   if(badChFlag == 1) {free(*readK); free(*readP);fprintf(stderr,"CLIENT: ERROR, input '%s' contains bad characters\n",pFile); exit(1);} // if the bad chat flag is set
   if(badChFlag == 2) {free(*readK); free(*readP);fprintf(stderr,"CLIENT: ERROR, input '%s' contains bad characters\n",kFile);exit(1);} // if the bad chat flag is set
   if(badChFlag == 3) {free(*readK); free(*readP);fprintf(stderr,"CLIENT: ERROR, input '%s and %s' contains bad characters\n",pFile, kFile);exit(1);} // if the bad chat flag is set
}

/********************************
 * This is the main function that drives the client code, it reads input, then process it and communicate it with the server
 * *****************************/
int main(int argc, char *argv[])
{

   char* pFile;
   char* kFile;
   char* pText=NULL;
   char* kText=NULL;
   char buffer[100000];
   int socketFD, portNumber, charsWritten, charsRead, totalCh;
   struct sockaddr_in serverAddress;
   struct hostent* serverHostInfo;

   if (argc < 4) { error("ERROR, wrong number of args\n"); } // Check usage & args
   pFile = argv[1];
   kFile = argv[2];

   checkFiles(pFile, kFile, &pText, &kText); // checks if the files are satisfy the encryption conditions

   // Set up the server address struct
   memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
   portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
   serverAddress.sin_family = AF_INET; // Create a network-capable socket
   serverAddress.sin_port = htons(portNumber); // Store the port number
   serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
   if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(2); }
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

   // Set up the socket
   socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (socketFD < 0) error("CLIENT: ERROR opening socket\n");

   // Connect to server
   if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
   {fprintf(stderr, "CLIENT: ERROR, could not contact otp_enc_d on port %d\n", portNumber); exit(2);}
   // Send message to server
   charsWritten = send(socketFD, "enc", 3, 0); // Write to the server
   if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");

   // Get return message from server
   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
   charsRead = recv(socketFD, buffer, 3, 0); // Read data from the socket, leaving \0 at end
   if (charsRead < 0) error("CLIENT: ERROR reading from socket\n");
   if (strcmp(buffer,"yes")!=0) {fprintf(stderr, "CLIENT: ERROR, could not contact otp_enc_d on port %d since it's assosiated with otp_dec_d\n", portNumber); exit(2);} // invalid communication

   // Send message size to server
   char sizeBuff[10];
   memset(sizeBuff, '\0', sizeof(sizeBuff)); // Clear out the buffer again for reuse
   snprintf(sizeBuff, 10, "%d", strlen(pText)); // convert it to a string
   charsWritten = send(socketFD, sizeBuff, sizeof(sizeBuff), 0); // Write to the server
   if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");
   // Send plain text to server
   totalCh=0;
   do{ // loops untill done sending
      //      printf("C:L1\n"); 
      if((strlen(pText)-totalCh) < 1000)
	 charsWritten = send(socketFD, (pText + totalCh), (strlen(pText)-totalCh), 0); // Write to the server
      else
	 charsWritten = send(socketFD, (pText + totalCh), 1000, 0); // Write to the server in chuncks of 1000 char
      if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");
      totalCh+=charsWritten; // increment offset
      //      printf("C1:**%d**\n", charsWritten);
   }while(totalCh < strlen(pText));

   // Send key text to server
   totalCh=0;
   do{ // loops untill done sending
      //     printf("C:L2\n"); 
      if((strlen(pText)-totalCh) < 1000)
	 charsWritten = send(socketFD, (kText + totalCh), (strlen(pText)-totalCh), 0); // Write to the server
      else
	 charsWritten = send(socketFD, (kText + totalCh), 1000, 0); // Write to the server in chuncks of 1000 chars
      if (charsWritten < 0) error("CLIENT: ERROR writing to socket\n");
      totalCh+=charsWritten; // increment offset
   }while(totalCh < strlen(pText));

   // Get decrypted text from server
   memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
   totalCh=0;
   do{ // loops untill done reading
      //     printf("C:L3\n"); 
      if((strlen(pText)-totalCh) < 1000)
	 charsRead = recv(socketFD, (buffer + totalCh), (strlen(pText)-totalCh), 0); // Read data from the socket in chuncks of 1000 chars
      else
	 charsRead = recv(socketFD, (buffer + totalCh), 1000, 0); // Read data from the socket in chuncks of 1000 chars
      if (charsRead < 0) error("CLIENT: ERROR reading from socket\n");
      totalCh+=charsRead; // inc. offset 
      //      printf("C2:**%d**\n", charsRead);
   }while(totalCh<strlen(pText));
   printf("%s\n",buffer); // print the txt to stdout

   close(socketFD); // Close the socket
   free(pText); // free memory
   free(kText); // free memory 
   return 0;
}
