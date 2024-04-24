#include "../headers/jobExecutor.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
extern int concurrency;

void issueJob(char* job) {

}

void setConcurrency(int n) {
    concurrency = n;
}

void jobStop(int jobId) {
    printf("Stopping process %d\n",jobId);
}

void print(int mode) {
    if (mode == 0) { // poll
    
    } else { //exit
    
    }
}

bool isUp() {
    printf("Establishing Connection with server...\n");
    int fd = open("jobExecutorServer.txt",O_RDONLY);
    if(fd) {
        printf("Listening on pid ");
        char pid[10];
        read(fd,pid,10);
        printf("%s\n",pid);
        close(fd);
    } else {
        printf("Cannot find Server file. Connection failed...\n");
        exit(1);
    }
}