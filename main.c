/* ----------------------------------------------------------- */
/* NAME : Eric Grant                         User ID: ewgrant  */
/* DUE DATE : 02/17/2021                                       */
/* PROGRAM ASSIGNMENT 2                                        */
/* FILE NAME : main.c                                          */
/* PROGRAM PURPOSE :                                           */
/*     concurrently run qsort and merge with shared memory     */
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
/*     sorry I did everything in main                          */
/* ----------------------------------------------------------- */
int main(){
    //print header
    printf("Quicksort and Binary Merge with Multiple Processes:\n\n");

    //vars
    int k, m, n;
    pid_t pid;
    char buf[1000], temp[1000], build[1000] = "";
    int status;

    //read in from file
    fscanf(stdin, "%10d", &k);
    int a[k];
    for (int i = 0; i<k; i++){
        fscanf(stdin, "%10d", &a[i]);
    }

    fscanf(stdin, "%10d", &m);
    int x[m];
    for (int i = 0; i<m; i++){
        fscanf(stdin, "%10d", &x[i]);
    }

    fscanf(stdin, "%10d", &n);
    int y[n];
    for (int i = 0; i<n; i++){
        fscanf(stdin, "%10d", &y[i]);
    }

    //declare shared memory
    key_t key = ftok("./",'h');
    printf("*** MAIN: shared memory key = %d\n", key);
    int shmId = shmget(key, (k + 2*m + 2*n) * sizeof(int), IPC_CREAT | 0666);
    printf("*** MAIN: shared memory created\n");
    int *data;
    data = (int*)shmat(shmId, NULL, 0);
    printf("*** MAIN: shared memory attached and is ready to use\n\n");

    //add data to shared memory
    for (int i = 0; i < k; i++) {
        data[i] = a[i];
    }

    for (int i = 0; i < m; i++) {
        data[i+k] = x[i];
    }

    for (int i = 0; i < n; i++) {
        data[i+k+m] = y[i];
    }

    for (int i = 0; i < m+n; i++) {
        data[i+k+m+n] = 0;
    }

    //print array info from shared memory
    printf("Input array for qsort has %d elements:\n    ", k);
    for (int i = 0; i < k; i++) {
        printf("%d  ",data[i]);
    }
    printf("\n\n");

    printf("Input array x[] for merge has %d elements:\n    ", m);
    for (int i = 0; i < m; i++) {
        printf("%d  ",data[i+k]);
    }
    printf("\n\n");

    printf("Input array y[] for merge has %d elements:\n    ", n);
    for (int i = 0; i < n; i++) {
        printf("%d  ",data[i+k+m]);
    }
    printf("\n\n");

    //spawn qsort
    char* argq[10];
    pid = fork();
    pid_t qsortPid = pid;
    if (pid > 0) {
        sprintf(buf, "*** MAIN: about to spawn the process for qsort\n");
        write(1, buf, strlen(buf));
    } else if (pid == 0) {
        argq[0] = "./qsort";
        argq[1] = "0";

        sprintf(temp, "%d", k-1);
        argq[2] = malloc(sizeof(char) * 10);
        strcpy(argq[2], temp);

        sprintf(temp, "%d", k);
        argq[3] = malloc(sizeof(char) * 10);
        strcpy(argq[3], temp);

        sprintf(temp, "%d", m);
        argq[4] = malloc(sizeof(char) * 10);
        strcpy(argq[4], temp);

        sprintf(temp, "%d", n);
        argq[5] = malloc(sizeof(char) * 10);
        strcpy(argq[5], temp);
        
        argq[6] = NULL;
        execvp(*argq, argq);
        exit(0);
    }

    //spawn merge
    char* argm[10];
    pid = fork();
    pid_t mergePid = pid;
    if (pid > 0) {
        sprintf(buf, "*** MAIN: about to spawn the process for merge\n");
        write(1, buf, strlen(buf));
    } else if (pid == 0) {
        argm[0] = "./merge";

        sprintf(temp, "%d", k);
        argm[1] = malloc(sizeof(char) * 10);
        strcpy(argm[1], temp);

        sprintf(temp, "%d", m);
        argm[2] = malloc(sizeof(char) * 10);
        strcpy(argm[2], temp);

        sprintf(temp, "%d", n);
        argm[3] = malloc(sizeof(char) * 10);
        strcpy(argm[3], temp);

        argm[4] = NULL;

        execvp(*argm, argm);
        exit(0);
    }

    //wait till dead child and act depending on which dies
    pid_t pp;
    for (int i = 0; i < 2; i++) {
        pp = waitpid(-1, &status, 0);
        if (pp == qsortPid) {
            sprintf(build, "*** MAIN: sorted array by qsort:\n    ");
            for (int i = 0; i < k; i++)
            {
                sprintf(buf, "%d  ", data[i]);
                strcat(build, buf);
            }
            strcat(build, "\n\n");
            write(1, build, strlen(build));
            strcpy(build, "");
        }
        if (pp == mergePid) {
            sprintf(build, "*** MAIN: merged array:\n    ");
            for (int i = k+m+n; i < k+2*m+2*n; i++)
            {
                sprintf(buf, "%d  ", data[i]);
                strcat(build, buf);
            }
            strcat(build, "\n\n");
            write(1, build, strlen(build));
            strcpy(build, "");
        }
    }

    //remove shared memory
    shmdt(data);
    printf("*** MAIN: shared memory successfully detached\n");
    shmctl(shmId, IPC_RMID, NULL);
    printf("*** MAIN: shared memory successfully removed\n");

    return 0;
}