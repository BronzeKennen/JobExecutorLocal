#include "../headers/jobExecutorServer.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h> 

int concurrency = 1;
int queue = 0;
int jobId = 1;

char* namedFifo = "bin/comfifo";
const char* filename = "jobExecutorServer.txt";

int serverInit() {
    int pid = getpid(); 

    int fp = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fp == -1) {
        printf("Error opening file. Attempting to restart...\n");
        remove(filename);
        serverInit();
        return 1;
    } 

    if (flock(fp, LOCK_EX) == -1) {
        perror("Error locking file");
        close(fp);
        return -1;
    }

    printf("\n--===Server is up on pid: %d===--\n",pid);
    char strpid[20];
    snprintf(strpid,sizeof(strpid),"%d\n",pid);
    int err = write(fp,strpid,strlen(strpid));
    flock(fp,LOCK_UN);
    close(fp);
}


void issueJob(char* job) {
    jProperties *new;
    new = malloc(sizeof(jProperties));
    char jobString[7];

    sprintf(jobString,"job_%d",jobId);

    new->jobId = malloc(sizeof(jobString)*sizeof(char));
    new->jobId = jobString;
    jobId++;
    
    new->job = malloc(sizeof(job));
    new->job = job;

    new->qPos = queue++;
    printf("Job has been queued up as\n<%s,%s,%d>\n",new->jobId,new->job,new->qPos);
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
int serverClose() {
    remove(filename);
    printf("\n--===Server has been terminated===--\n");
    return 0;
}

int main(int argc, char** argv) {

    //Exits are for test purposes
    serverInit();
    int fd;
    char buf[100];
    fd = open(namedFifo,O_RDONLY);
    while(1) {
        read(fd,buf,100);
        if(strncmp(buf,"1",1) == 0) {
            printf("Server is shutting down...\n");
            break;
        } else if(strncmp(buf,"issueJob",8) == 0) {
            issueJob(buf+8);
            break;
        } else if(strncmp(buf,"setConcurrency",14) == 0) {
           int con = atoi(buf+14); 
           if(!con) {
                printf("Error reading argument. Exiting\n");
                break;
           }
           setConcurrency(con);
           break;
        }
    }
    serverClose();
    return 0;
}