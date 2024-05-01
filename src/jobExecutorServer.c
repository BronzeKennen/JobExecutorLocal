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

int concurrency = 1;
int queue = 0;
int jobId = 1;

char* namedFifo = "/tmp/comfifo";
const char* filename = "jobExecutorServer.txt";


// Table with all processes (RUNNING/QUEUED)
// Stack with queued processes only (POP ONLY FIRST)

pQueue* procTable;
int serverInit(int shmid) {
    int pid = getpid(); 

    int fp = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fp == -1) {
        printf("Error opening file. Attempting to restart...\n");
        remove(filename);
        serverInit(shmid);
        return 1;
    } 

    if (flock(fp, LOCK_EX) == -1) {
        perror("Error locking file");
        close(fp);
        return -1;
    }

    procTable = malloc(sizeof(pQueue));
    procTable->size = 0;
    procTable->first = NULL;
    procTable->last = NULL;

    printf("\n--===Server is up on pid: %d===--\n",pid);
    char strpid[20];
    snprintf(strpid,sizeof(strpid)+sizeof(shmid),"%d\n%d",pid,shmid);
    printf("%s\n",strpid);
    int err = write(fp,strpid,strlen(strpid));
    flock(fp,LOCK_UN);
    close(fp);
}


void issueJob(char* job) {
    if(procTable->first) printf("before: %s\n",procTable->first->data->job);
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
    if(procTable->first == NULL) {
        procTable->first = malloc(sizeof(pqNode));
        procTable->first->data = new;
    } else if(procTable->last == NULL) {
        procTable->last = malloc(sizeof(pqNode));
        procTable->last->data = new;
    } else {    
        procTable->last->next = malloc(sizeof(pqNode));
        procTable->last = procTable->last->next;
        procTable->last->data = new;
    }
    // printf("%p\n",new);
    // printf("%p\n",procTable->first->data);
    procTable->size++;
    pqNode *testProc = procTable->first;
    for(int i = 0; i < procTable->size; i++) {
        printf("->%d\n",procTable->size);
        printf("%s\n",testProc->data->job);
        testProc = testProc->next;
        if(testProc == NULL) break;
    }
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
    sem_t *semProc1;
    int shmidA;
    if ((shmidA = shmget(IPC_PRIVATE, sizeof(sem_t), (S_IRUSR|S_IWUSR))) == -1) { 
        perror("Failed to create shared memory segment");
        return 1;
    }
    if ((semProc1 = shmat(shmidA, NULL, 0)) == (void *)-1) {
        perror("Failed to attach memory segment");
        return 1;
    }

    serverInit(shmidA);
    //Initialize Semaphores
    if (sem_init(semProc1, 1, 1) == -1) {
        perror("Failed to initialize semaphore");
    }

    sem_wait(semProc1);
    int fd;
    char buf[100]; //will change later
    char compbuf[100];
    strcpy(compbuf,"\0");
    fd = open(namedFifo,O_RDONLY);
    while(1) {
        read(fd,buf,100);
        if(strncmp(buf,compbuf,strlen(buf)) == 0) {
            continue;
        }
        strcpy(compbuf,buf);
        if(strncmp(buf,"1",1) == 0) {
            printf("Server is shutting down...\n");
            break;
        } else if(strncmp(buf,"issueJob",8) == 0) {
            issueJob(buf+8);
        } else if(strncmp(buf,"setConcurrency",14) == 0) {
           int con = atoi(buf+14); 
           if(!con) {
                printf("Error reading argument. Exiting\n");
                break;
           }
           setConcurrency(con);
        }
        memset(buf,0,100);
        memset(compbuf,0,100);
    }
    serverClose();
    return 0;
}