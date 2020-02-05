################################################
# Author: Ibrahim Alarifi
# Date: 07/29/2019
# Description: This program generates three sets of random lowercase chars, and print them into 
#              text files and the monitor. Picks two random numbers [1,42] and multiply them.
# Interpreter: Python3
###############################################

import sys
import random
import string

##############################################
# This function generate a string of ten random chars and return it.
# Input: -
# Output: myst (string of chars)
##############################################
def randLetter():
   buff=random.choice(string.ascii_lowercase); # pick a random letter from a pre-define list
   myst=buff; # put it in myst
   for i in range(0,9): # loops 9 times
      buff=random.choice(string.ascii_lowercase); # pick a random letter from a pre-define list
      myst += buff # cat it into myst
   myst += "\n" # add a new line at the end
   return myst # return myst
 
#############################################
# This is the main function of the program
############################################ 
file = open("fileOne", "w+") # open the 1st text file
letterString1=randLetter(); # call randLetter to the string
file.write(letterString1); # write that string in the txt file 
file.close(); # close the txt file
   
file = open("fileTwo", "w+") # 2nd txt file, uses same presedure as the 1st one
letterString2=randLetter();
file.write(letterString2);
file.close()

file = open("fileThree", "w+") # 3rd txt file uses same presedure as the 1st one
letterString3=randLetter();
file.write(letterString3);
file.close()

x = random.randint(1,43); # pick a random number from [1,42]
y = random.randint(1,43); # pick a random number from [1,42]
z = x*y # multiply them together

sys.stdout.write(letterString1) # print 1st txt file content
sys.stdout.write(letterString2) # print 2nd txt file content
sys.stdout.write(letterString3) # print 3rd txt file content
print(x) # print 1st number
print(y) # print 2nd number
print(z) # print their multiplication 
