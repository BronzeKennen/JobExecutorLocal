#include "../headers/jobExecutorServer.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h> 
#include <sys/file.h>
#include <sys/wait.h>
#include <signal.h>

int concurrency = 1;
int queue = 0;
int jobId = 1;

char* namedFifo = "/tmp/comfifo";
char* execFifo = "/tmp/execfifo";
const char* filename = "jobExecutorServer.txt";


// Table with all processes (RUNNING/QUEUED)
// Stack with queued processes only (POP ONLY FIRST)

pQueue procTable;
pQueue qInit() {
    pQueue q = malloc(sizeof(*q));
    q->size = 0;
    q->first = NULL;
    q->last = NULL;
}


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
    char strpid[20];
    snprintf(strpid,sizeof(strpid),"%d",pid);
    int err = write(fp,strpid,strlen(strpid));
    flock(fp,LOCK_UN);


    printf("\n--===Server is up on pid: %d===--\n",pid);
    close(fp);
    return 0;
}


void issueJob(char* job) {
    jProperties *new = malloc(sizeof(jProperties));
    char jobString[7];
    sprintf(jobString,"job_%d",jobId);
    new->jobId = malloc(strlen(jobString));
    strncpy(new->jobId,jobString,strlen(jobString));
    jobId++;
    new->job = malloc(strlen(job));
    strncpy(new->job,job,strlen(job));
    new->qPos = queue++;

    if(procTable->first == NULL) {
        procTable->first = malloc(sizeof(*procTable->first));
        procTable->first->data = malloc(sizeof(jProperties));
        procTable->first->data = new;
        procTable->last = malloc(sizeof(*procTable->last));
        procTable->last->data = malloc(sizeof(jProperties));
        procTable->last->data = new;
        //Initialize first and last nodes
    } else if(procTable->first->data->qPos == procTable->last->data->qPos) {
        procTable->last->data = new;
        procTable->first->next = malloc(sizeof(*procTable->first->next));
        procTable->first->next = procTable->last;
        //second element is last on list, and next one after first
    } else if(procTable->last->next == NULL) {    
        procTable->last->next = malloc(sizeof(*procTable->last->next));
        procTable->last->next->data = new;
        procTable->last = procTable->last->next;
        //whichever element enters is now last, therefore next of the previous last
    }
    //debug
    // pqNode s = procTable->first;
    // printf("%s,%s,%d\n",s->data->job,s->data->jobId,s->data->qPos);
    // if(s) {
        // for(int i = 0; i < queue; i++) {
            // printf("%d: %s,%s,%d\n",i,s->data->job,s->data->jobId,s->data->qPos);
            // s = s->next;
            // if(!s) break;
        // }
    // }
}

void runQueueItem() {
    pid_t pid;
    int fd;
    pid = fork();
    if(pid != 0) return;
    printf("child proc\n");
    char buffer[100];
    fd = open(execFifo,O_RDONLY | O_NONBLOCK);
    read(fd,buffer,100);
    printf("->%s\n",buffer);
    exit(0);
    
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
    exit(0);
}

int execfd;
    int fd;
void sig_handler(int signo) {
    char buf[100]; //will change later
    if(signo == SIGUSR1) {
        read(fd,buf,100);
        if(strncmp(buf,"1",1) == 0) {
            printf("Server is shutting down...\n");
            serverClose();
        } else if(strncmp(buf,"issueJob",8) == 0) {
            issueJob(buf+8);
            write(execfd,buf+8,strlen(buf+8));
            runQueueItem();
            wait(NULL);
        } else if(strncmp(buf,"setConcurrency",14) == 0) {
            int con = atoi(buf+14); 
            if(!con) {
                printf("Error reading argument. Exiting\n");
            }
            setConcurrency(con);
        }
    }
}

int main(int argc, char** argv) {
    mkfifo(execFifo,0666);
    execfd = open(execFifo,O_WRONLY | O_NONBLOCK);

    procTable = qInit();

    if (signal(SIGUSR1, sig_handler) == SIG_ERR) { // Here, we set sig_handler as the handler for SIGUSR1
        printf("Error registering signal handler\n");
        return 1;
    }
    fd = open(namedFifo,O_RDONLY | O_NONBLOCK);
    serverInit();
    while(1) {

    }
    return 0;
}