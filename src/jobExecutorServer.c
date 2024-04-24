#include "../headers/jobCommander.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int serverInit() {
    char* filename = "jobExecutorServer.txt";
    int pid = getpid(); 

    int fp = open(filename,O_WRONLY | O_CREAT);
    if(!fp) {
        printf("Error opening file. \n");
        return -1;
    } 

    printf("\n--===Server is up on pid: %d===--\n",pid);
    char strpid[20];
    sprintf(strpid,"%d",pid);
    char* buf = malloc(sizeof(strlen(strpid)));
    sprintf(buf,"%d",pid);
    write(fp,buf,sizeof(buf));
    close(fp);
    free(buf);
}

int serverClose() {
    remove("jobExecutorServer.txt");
    printf("\n--===Server has been terminated===--\n");
    return 0;
}