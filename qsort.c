/* ----------------------------------------------------------- */
/* NAME : Eric Grant                         User ID: ewgrant  */
/* DUE DATE : 02/17/2021                                       */
/* PROGRAM ASSIGNMENT 2                                        */
/* FILE NAME : qsort.c                                         */
/* PROGRAM PURPOSE :                                           */
/*     sort a list in shared memory using recursive processes  */
/* ----------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <errno.h>

/* ----------------------------------------------------------- */
/* FUNCTION  main :                                            */
/*     create shared memory linked to an array of ints         */
/*     create children and have them run qsort and merge       */
/*     print the updated shared data after the children finish */
/*     sorry I did everything in main :^)                      */
/* PARAMETER USAGE :                                           */
/*    argc is the total number of arguments taken              */
/*    argv: 1= left end of array, 2= right end of array        */
/*    3= length of array 0, 4= length of array 1               */
/*    5= length of array 2,                                    */
/* ----------------------------------------------------------- */
int main(int argc, char* argv[]){
    //get command line input
    if (argc != 6) {
        return 0;
    }

    int l = atoi(argv[1]);
    int r = atoi(argv[2]);
    int k = atoi(argv[3]);
    int m = atoi(argv[4]);
    int n = atoi(argv[5]);

    //exit if end case
    if (l >= r) {
        return 0;
    }
    
    //vars
    pid_t myPid = getpid(), pid;
    int status, hold;
    char buf[1000], temp[1000], build[1000] = "";

    //declare shared memory
    key_t key = ftok("./",'h');
    int shmId = shmget(key, (k + 2*m + 2*n) * sizeof(int), 0666);
    int *data;
    data = (int*)shmat(shmId, NULL, 0);

    //print start
    sprintf(build, "   ### Q-PROC(%d): entering with a[%d..%d]\n       ", myPid, l, r);
    for (int i = l; i<r+1; i++) {
        sprintf(buf, "%d  ", data[i]);
        strcat(build, buf);
    }
    strcat(build, "\n");
    write(1, build, strlen(build));
    strcpy(build, "");

    //quick sort using right index as pivot
    int i = l-1;
    int j = l;
    int p = r;
    while (j < p) {
        if (data[j] < data[p]) {
            i++;
            hold = data[j];
            data[j] = data[i];
            data[i] = hold;
        }
        j++;
    }
    i++;
    hold = data[p];
    data[p] = data[i];
    data[i] = hold;

    sprintf(buf, "   ### Q-PROC(%d): pivot element is a[%d] = %d\n", myPid, i, data[i]);
    write(1, buf, strlen(buf));

    //print sorted
    sprintf(build, "   ### Q-PROC(%d): section a[%d..%d] sorted\n       ", myPid, l, r);
    for (int i = l; i<r+1; i++) {
        sprintf(buf, "%d  ", data[i]);
        strcat(build, buf);
    }
    strcat(build, "\n");
    write(1, build, strlen(build));
    strcpy(build, "");

    //spawn qsort left
    char* argl[10];
    pid = fork();
    if (pid == 0) {
        argl[0] = "./qsort";
        argl[1] = argv[1];
        
        sprintf(temp, "%d", i-1);
        argl[2] = malloc(sizeof(char) * 10);
        strcpy(argl[2], temp);

        argl[3] = argv[3];
        argl[4] = argv[4];
        argl[5] = argv[5];
        argl[6] = NULL;
        int test = execvp(*argl, argl);
        exit(0);
    }

    //spawn qsort right
    char* argr[10];
    pid = fork();
    if (pid == 0) {
        argr[0] = "./qsort";
        
        sprintf(temp, "%d", i+1);
        argr[1] = malloc(sizeof(char) * 10);
        strcpy(argr[1], temp);

        argr[2] = argv[2];
        argr[3] = argv[3];
        argr[4] = argv[4];
        argr[5] = argv[5];
        argr[6] = NULL;
        int test = execvp(*argr, argr);
        exit(0);
    }

    //wait till over
    for (int i = 0; i < 2; i++) {
        wait(&status);
    }

    //exit and detach
    shmdt(data);
    sprintf(buf, "   ### Q-PROC(%d): exits\n", myPid);
    write(1, buf, strlen(buf));

    return 0;
}