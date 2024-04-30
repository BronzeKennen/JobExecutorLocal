#include "../headers/jobExecutorServer.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int concurrency = 1;
int queue = 0;
int jobId = 1;
char* namedFifo = "bin/comfifo";
int serverInit() {
    //Sometimes opening the file fails dk why doesn't throw an error
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
    remove("jobExecutorServer.txt");
    printf("\n--===Server has been terminated===--\n");
    return 0;
}

int main(int argc, char** argv) {

    //Exits are for test purposes
    int fd;
    char buf[100];
    fd = open(namedFifo,O_RDONLY);
    while(1) {
        read(fd,buf,100);
        if(strncmp(buf,"1",1) == 0) {
            printf("Server is shutting down...\n");
            exit(0);
        } else if(strncmp(buf,"issueJob",8) == 0) {
            issueJob(buf+8);
            exit(0);
        } else if(strncmp(buf,"setConcurrency",14) == 0) {
           int con = atoi(buf+14); 
           if(!con) {
                printf("Error reading argument. Exiting\n");
                exit(1);
           }
           setConcurrency(con);
           exit(0);
        }
    }
    return 0;
}