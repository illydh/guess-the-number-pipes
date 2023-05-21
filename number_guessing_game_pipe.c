#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

int main(int argc, char* argv[])
{
    //  create pipes for parent and child to communicate
    int fdp[2], fdc[2];         

    if (pipe(fdp)==-1 || pipe(fdc)==-1)         //  case if pipe() fails
    {
        perror("pipe()");
        exit(EXIT_FAILURE);
    }


    //  fork to create multiple processes
    pid_t pid = fork();

    if (pid<0)      //  case if fork fails
    {
        perror("fork()");
        exit(EXIT_FAILURE);
    }


    //  child process
    if (pid==0)         //  process ID to generate number
    {
        srand(time(NULL));          //  generate number
        
        int number = (rand() % 100) + 1;        //  random integer between 1-100
        int attempts = 0;           //  number of attempt(s)
        
        //  initial msg
        char msg[100]; 
        strcpy(msg, "Wrong!\n");        

        // close unused file descriptors
        close(fdc[0]);
        close(fdp[1]);
        
        //  tell the other process ID what is the max value the answer could be
        int max = 100;
        write(fdc[1], &max, sizeof(int));

        int guess;
        int result=1;

        //  keep checking if the guesser is correct
        while (result!=0)
        {
            read(fdp[0], &guess, sizeof(int));      //  receive guess
            attempts++;     // increment number of attempt(s)
            
            if(number==guess) {     //  correct!
                snprintf(msg, 100, "The answer was %d. It took %d attempt(s).\n", number, attempts);
                result = 0;
            } else if (number > guess) {       
                result = 1;
            } else {
                result = -1;
            }
            write(fdc[1], &result, sizeof(int));        //  tell the guesser the outcome
        }
        write(fdc[1], msg, (strlen(msg)+1));        //  write back the final msg

        //  close used descripters
        close(fdc[1]);
        close(fdp[0]);

        exit(EXIT_SUCCESS);     //  kill process ID once complete
    }


    //  parent process
    //  guesser process starts here
    int min = 1;
    int max, guess, result;

    //  close unused descriptors
    close(fdp[0]);
    close(fdc[1]);
    
    read(fdc[0], &max, sizeof(int));    //  receive the max possible value for answer

    //  guess
    //  implements binary search algorithm
    do {
        guess = (min+max) / 2;      //  midpoint
        printf("Guess: %d\n", guess);       //  record guess

        write(fdp[1], &guess, sizeof(int));     //  are we correct or incorrect?
        read(fdc[0], &result, sizeof(int));     //  receive decision

        if (result>0)       //  incorrect and the answer is greater than the guess
            min=guess+1;        //  recurse thru upper half of possible values

        else if (result<0)      //  incorrect and the answer is less than the guess
            max=guess-1;        //  recurse thru lower half of possible values

    } while (result!=0);    

    fflush(stdout);

    //  get final msg
    char msg;
    read(fdc[0], &msg, sizeof(char));
    while (msg!='\0') 
    {
        write(1, &msg, sizeof(char));
        read(fdc[0], &msg, sizeof(char));
    }
    
    //  close used descriptors
    close(fdc[0]);
    close(fdp[1]);

    //  wait for all other processes to finish
    wait(NULL);
    
    return 0;
}