/***********************************
 * Author: Ibrahim Alarifi
 * Date: 08/11/2019
 * Discreption: This is code takes input from user of the length of the key, then generate a key of random chars to stdout
 * Output: n random chars (higher-case an space)
 * Input: n number of chars to be generated
 **********************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <sys/stat.h>
#include <unistd.h>


/********************************
 * This is the main function that drive the chars generation
 *******************************/
int main (int argc, char *argv[]){

   char *keyArr= NULL;
   int i, buff;

   if (argc < 2) { fprintf(stderr, "ERROR, wrong number of args\n"); exit(1); } // Check usage & args
   
   srand(time(NULL)); // random seed
   int nKeys = atoi(argv[1]); // gets the number of chars
   keyArr = (char*)malloc( nKeys * sizeof(char)); // allocate memory for it
   for(i=0;i<nKeys;i++){ // this loop generate n number of random chars
      buff = (rand() % (90 - 65)) + 64; // randomize a number in [64-90] i.e. (from " " to Z)
      if (buff == 64) buff = 32; // re-map space to its real value 
      keyArr[i]= (char) buff; // conver the value to a char and store it in the array
   }

   for(i=0;i<nKeys;i++) // this loop prints the chars to stdout
      printf("%c", keyArr[i]);
   printf("\n");

   free(keyArr); // free memory 
   return 0;
}
