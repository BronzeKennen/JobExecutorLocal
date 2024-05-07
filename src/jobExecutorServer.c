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
int running = 0;
int jobId = 1;

char* namedFifo = "/tmp/comfifo";
char* execFifo = "/tmp/execfifo";
const char* filename = "jobExecutorServer.txt";

int execfd;
int fd;

// Table with all processes (RUNNING/QUEUED)
// Stack with queued processes only (POP ONLY FIRST)

pQueue procTable;
pQueue qInit() {
    pQueue q = malloc(sizeof(*q));
    q->size = 0;
    q->first = NULL;
    q->last = NULL;
}

void pqPopFirst() {
    free(procTable->first->data);
    if(procTable->first->next) {
        procTable->first = procTable->first->next;
    } else {
        free(procTable->first);
        procTable->first = NULL;
    }
}

pqNode pqFindJob(char* jobId) {
    pqNode node = procTable->first;
    for(int i = 0; i < procTable->size; i++) {
        if(node->data->jobId == jobId) return node;
        node = node->next;

    }
}

void pqRemove(pqNode node) {
    pqNode f = procTable->first;
    if(node == f) pqPopFirst;
    pqNode s = procTable->first->next;
    for(int i = 0; i < procTable->size-1; i++) {
        if(s == node) {
            f->next = s->next;
        }
        f = f->next;
        s = s->next;
    }
}

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
    char strpid[20];
    snprintf(strpid,sizeof(strpid),"%d\n%d",pid,shmid);
    int err = write(fp,strpid,strlen(strpid));
    flock(fp,LOCK_UN);


    printf("\n--===Server is up on pid: %d===--\n",pid);
    close(fp);
    return 0;
}

void setConcurrency(int n) {
    concurrency = n;
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
    printf("Job has been issued on queue with id: %s\n",new->jobId);
    procTable->size++;
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
void runQueueItem(pid_t* pid) {

    *pid = fork();
    if(*pid == -1) {
        printf("fork\n");
        exit(EXIT_FAILURE);
    }
    if(*pid != 0) return;

    char buffer[100];
    char* args[100];
    memset(args,0,100);
    memset(buffer,0,100);
    int fd;
    fd = open(execFifo,O_RDONLY);
    if(fd == -1) {
        printf("open error\n");
        exit(EXIT_FAILURE); 
    }
    read(fd,buffer,100);

    char* token = strtok(buffer," ");
    int i = 0;
    while(token) {
        args[i++] = token;
        token = strtok(NULL," ");

    }
    args[i] = NULL;
    execvp(args[0],args);

    perror("execvp");
    exit(EXIT_FAILURE);
    
}

void executionCheck() {
    int status;
    pid_t pid;
    if(!queue) return;
    if(running < concurrency) {
        pqNode toExec = procTable->first;
        write(execfd,toExec->data->job,strlen(toExec->data->job));
        printf("run Proc %d|con %d\n",running++,concurrency);

        runQueueItem(&pid);
    } 
}


void jobStop(char* jobId) {
    pqNode toFind = pqFindJob(jobId);
    printf("%s,%s,%d\n",toFind->data->job,toFind->data->jobId,toFind->data->qPos);

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


void chld_handler(int signo) {
    if(signo == SIGCHLD) {
        printf("Job finished!\n");
        queue--;
        running--;
        pqPopFirst();
        executionCheck();
    }
}


int main(int argc, char** argv) {
    int shmidA;
    sem_t *semProc1;
    if ((shmidA = shmget(IPC_PRIVATE, sizeof(sem_t)*2 + 15, (S_IRUSR|S_IWUSR))) == -1) { //desmevw 50 theseis se afti ti periptwsi char gia testing
        perror("Failed to create shared memory segment");
        return 1;
    } else {
        printf("shmidA: %d\n",shmidA);
    }
    serverInit(shmidA);
    if ((semProc1 = shmat(shmidA, NULL, 0)) == (void *)-1) {
        perror("Failed to attach memory segment");
        return 1;
    }
    if (sem_init(semProc1, 1, 0) == -1) {
        perror("Failed to initialize semaphore");
    } else {
        printf("Created semProc1\n");
    }

    mkfifo(execFifo,0666);
    execfd = open(execFifo,O_RDWR | O_NONBLOCK);
    procTable = qInit();
    // if (signal(SIGUSR1, sig_handler) == SIG_ERR) { 
        // printf("handler error\n");
        // return 1;
    // }
    if (signal(SIGCHLD, chld_handler) == SIG_ERR) { 
        printf("handler error\n");
        return 1;
    }
    fd = open(namedFifo,O_RDONLY | O_NONBLOCK);
    char buf[100]; //will change later
    while(1) {
        sem_wait(semProc1);
        memset(buf,0,100);
        read(fd,buf,100);
        printf("%s\n",buf);
        if(strncmp(buf,"1",1) == 0) {
            printf("Server is shutting down...\n");
            serverClose();
        } else if(strncmp(buf,"issueJob",8) == 0) {
            issueJob(buf+8);
            executionCheck();
        } else if(strncmp(buf,"setConcurrency",14) == 0) {
            int con = atoi(buf+14); 
            if(!con) {
                printf("Error reading argument. Exiting\n");
            }
            setConcurrency(con);
            executionCheck();
        } else if(strncmp(buf,"stop",4) == 0) {
            jobStop(buf+4);            
        }
    }
    return 0;
}