/* ----------------------------------------------------------- */
/* NAME : Eric Grant                         User ID: ewgrant  */
/* DUE DATE : 02/17/2021                                       */
/* PROGRAM ASSIGNMENT 2                                        */
/* FILE NAME : merge.c                                         */
/* PROGRAM PURPOSE :                                           */
/*     merge two sorted arrays using shared memory             */
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

/* ----------------------------------------------------------- */
/* FUNCTION  main :                                            */
/*     take two sorted arrays from shared memory, merge them   */
/*     using modified binary search                            */
/* PARAMETER USAGE :                                           */
/*    argc is the total number of arguments taken              */
/*    argv: 1= length of array 0, 2= length of array 1         */
/*    3= length of array 2                                     */
/* ----------------------------------------------------------- */
int main(int argc, char* argv[]){
    //get command line input
    if (argc != 4) {
        return 0;
    }

    int k = atoi(argv[1]);
    int m = atoi(argv[2]);
    int n = atoi(argv[3]);

    //vars
    char buf[1000];
    pid_t myPid = getpid(), pid;
    int status, km = (k+m), kmn = (k+m+n), index;

    //declare shared memory
    key_t key = ftok("./",'h');
    int shmId = shmget(key, (k + 2*m + 2*n) * sizeof(int), 0666);
    int *data;
    data = (int*)shmat(shmId, NULL, 0);

    //run x loop
    for (int i = 0; i < m; i++){
        pid = fork();
        if (pid == 0) {
            sprintf(buf, "      $$$ M-PROC(%d): handling x[%d] = %d\n", myPid, i, data[k+i]);
            write(1, buf, strlen(buf));

            if (data[k+i] < data[km]) {
                sprintf(buf, "      $$$ M-PROC(%d): x[%d] = %d is found to be smaller than y[%d] = %d\n", myPid, i, data[k+i], 0, data[km]);
                write(1, buf, strlen(buf));

                sprintf(buf, "      $$$ M-PROC(%d): about to write x[%d] = %d into position %d of the output array\n", myPid, i, data[k+i], i);
                write(1, buf, strlen(buf));

                data[kmn+i] = data[k+i];
            } else if (data[k+i] > data[kmn-1]) {
                sprintf(buf, "      $$$ M-PROC(%d): x[%d] = %d is found to be greater than y[%d] = %d\n", myPid, i, data[k+i], n, data[kmn-1]);
                write(1, buf, strlen(buf));

                sprintf(buf, "      $$$ M-PROC(%d): about to write x[%d] = %d into position %d of the output array\n", myPid, i, data[k+i], i+n);
                write(1, buf, strlen(buf));

                data[kmn+i+n] = data[k+i];
            } else {
                index = 0;
                do {
                    index++;
                } while (data[km+index] < data[k+i]);
                
                sprintf(buf, "      $$$ M-PROC(%d): x[%d] = %d is found between y[%d] = %d and y[%d] = %d\n", myPid, i, data[k+i], index-1, data[km+index-1], index, data[km+index]);
                write(1, buf, strlen(buf));

                sprintf(buf, "      $$$ M-PROC(%d): about to write x[%d] = %d into position %d of the output array\n", myPid, i, data[k+i], i+index);
                write(1, buf, strlen(buf));
                
                data[kmn+i+index] = data[k+i];
            }
            exit(0);
        }
    }

    //run y loop
    for (int i = 0; i < n; i++){
        pid = fork();
        if (pid == 0) {
            sprintf(buf, "      $$$ M-PROC(%d): handling y[%d] = %d\n", myPid, i, data[k+i]);
            write(1, buf, strlen(buf));

            if (data[km+i] < data[km]) {
                sprintf(buf, "      $$$ M-PROC(%d): y[%d] = %d is found to be smaller than x[%d] = %d\n", myPid, i, data[km+i], 0, data[k]);
                write(1, buf, strlen(buf));

                sprintf(buf, "      $$$ M-PROC(%d): about to write y[%d] = %d into position %d of the output array\n", myPid, i, data[km+i], i);
                write(1, buf, strlen(buf));

                data[kmn+i] = data[km+i];
            } else if (data[km+i] > data[km-1]) {
                sprintf(buf, "      $$$ M-PROC(%d): y[%d] = %d is found to be greater than x[%d] = %d\n", myPid, i, data[km+i], m, data[km-1]);
                write(1, buf, strlen(buf));

                sprintf(buf, "      $$$ M-PROC(%d): about to write y[%d] = %d into position %d of the output array\n", myPid, i, data[km+i], i+m);
                write(1, buf, strlen(buf));

                data[kmn+i+m] = data[km+i];
            } else {
                index = 0;
                do {
                    index++;
                } while (data[k+index] < data[km+i]);
                
                sprintf(buf, "      $$$ M-PROC(%d): y[%d] = %d is found between x[%d] = %d and x[%d] = %d\n", myPid, i, data[km+i], index-1, data[k+index-1], index, data[k+index]);
                write(1, buf, strlen(buf));

                sprintf(buf, "      $$$ M-PROC(%d): about to write y[%d] = %d into position %d of the output array\n", myPid, i, data[km+i], i+index);
                write(1, buf, strlen(buf));
                
                data[kmn+i+index] = data[km+i];
            }
            exit(0);
        }
    }

    //wait till over
    for (int i = 0; i < m+n; i++) {
        wait(&status);
    }

    //exit and detach
    shmdt(data);
    sprintf(buf, "      $$$ M-PROC(%d): exits\n", myPid);
    write(1, buf, strlen(buf));
    
    return 0;
}