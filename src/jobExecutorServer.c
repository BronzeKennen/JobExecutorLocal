#include "../headers/jobExecutorServer.h"
#include "../headers/pq.h"
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

sem_t *semProc2;
pqNode pqFindJob(char* jobId, pQueue q) {
    pqNode node = q->first;
    jProperties* data = (jProperties*)node->data;
    char* tok = strtok(jobId+1," ");
    for(int i = 0; i < q->size; i++) {
        if(strncmp(jobId+1,data->jobId,strlen(jobId)) == 0) return node;
        node = node->next;

    }
}

pQueue runningProcs;
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
    fflush(stdout);
    close(fp);
    return 0;
}

void setConcurrency(int n) {
    concurrency = n;
}

void pqPrint(pQueue q,int mode) {
    if(!mode) {
        printf("PRINTING RUNNING JOBS\n");
        pqNode s = q->first;
        runningJobs* data = (runningJobs*)s->data;
        printf("0: %s,%d\n",data->jobId,data->pid);
        if(s) {
            for(int i = 1; i < queue; i++) {
                s = s->next;
                if(!s) break;
                data = (runningJobs*)s->data;
                printf("%d: %s,%d\n",i,data->jobId,data->pid);
            }
        }
    } else{
        printf("PRINTING QUEUED JOBS\n");
        pqNode s = q->first;
        jProperties* data = (jProperties*)s->data;
        printf("0, %s,%s,%d\n",data->job,data->jobId,data->qPos);
        if(s) {
            for(int i = 1; i < queue; i++) {
                s = s->next;
                if(!s) break;
                data = (jProperties*)s->data;
                printf("%d: %s,%s,%d\n",i,data->job,data->jobId,data->qPos);
            }
        }
    }
}

void insert(void* new, pQueue q) {

    printf("inserting\n");
    if(q->first == NULL) {
        q->first = malloc(sizeof(*q->first));
        q->first->data = malloc(sizeof(void*));
        q->first->data = new;
        q->last = malloc(sizeof(*q->last));
        q->last->data = malloc(sizeof(void*));
        q->last->data = new;
        //Initialize first and last nodes
    } else if(q->size == 1) {
        q->last->data = new;
        q->first->next = malloc(sizeof(*q->first->next));
        q->first->next = q->last;
        //second element is last on list, and next one after first
    } else  {    
        q->last->next = malloc(sizeof(*q->last->next));
        q->last->next->data = new;
        q->last = q->last->next;
        //whichever element enters is now last, therefore next of the previous last
    }
    q->size++;
}

void issueJob(char* job,pQueue q) {
    queue++;
    jProperties *new = malloc(sizeof(jProperties));
    char jobString[7];
    sprintf(jobString,"job_%d",jobId);
    new->jobId = malloc(strlen(jobString));
    strncpy(new->jobId,jobString,strlen(jobString));
    new->qPos = jobId;
    jobId++;
    new->job = malloc(strlen(job));
    strncpy(new->job,job,strlen(job));
    insert(new,q);
    printf("Job %s has been issued on queue with id: %s\n",new->job,new->jobId);
}
void runQueueItem(pid_t* pid) {
    int shmid;
    *pid = fork();
    if(*pid == -1) {
        printf("fork\n");
        exit(EXIT_FAILURE);
    }
    if(*pid != 0) return;
    if(access(filename,F_OK) != -1) {
        int fd = open(filename,O_RDONLY);
        char lmao[20];
        read(fd,lmao,20);
        char* token = strtok(lmao,"\n");
        token = strtok(NULL,"\n");
        shmid = atoi(token);
    }
    semProc2 = shmat(shmid,NULL,0);
    if (semProc2 == (void *) -1) {
		perror("Attachment."); 
		exit(2);
	}
    semProc2++;

    char buffer[100];
    char* args[100];
    memset(args,0,100);
    memset(buffer,0,100);
    int fd = open(execFifo,O_RDONLY);
    if(fd == -1) {
        printf("open error\n");
        exit(EXIT_FAILURE); 
    }
    sem_wait(semProc2);
    read(fd,buffer,100);
    close(fd);
    printf("TO RUN: %s\n",buffer);

    char* tok = strtok(buffer," ");
    int i = 0;
    while(tok) {
        args[i++] = tok;
        tok = strtok(NULL," ");

    }
    args[i] = NULL;
    execvp(args[0],args);

    perror("execvp");
    exit(EXIT_FAILURE);
    
}

void executionCheck(pQueue q, pQueue curRunning) {

    int status;
    pid_t pid;
    if(!queue) return;
    if(running < concurrency) {
        pqNode toExec = pqGetFirst(q);
        if(!toExec) return;

        jProperties *data = (jProperties*)toExec->data;
        runningJobs *p =malloc(sizeof(runningJobs));
        p->jobId = malloc(strlen(data->jobId));
        strncpy(p->jobId,data->jobId,strlen(data->jobId));
        pqPopFirst(q);
        insert(p,curRunning);
        running++;
        write(execfd,data->job+1,strlen(data->job+1));
        sem_post(semProc2);
        runQueueItem(&pid);
        p->pid = pid;
        queue--;
    } 
}


void jobStop(char* jobId) {
    // printf("CALLED JOB STOP!\n");
    // pqNode toFind = pqFindJob(jobId);
    // if(toFind) {
        // printf("%s,%s,%d JOB STOP FINDINGS\n",toFind->data->job,toFind->data->jobId,toFind->data->qPos);
    // }
    // 
    // pqRemove(toFind);

}

int serverClose() {
    remove(filename);
    printf("\n--===Server has been terminated===--\n");
    exit(0);
}

int main(int argc, char** argv) {
    pQueue queuedProcs;
    pQueue runningProcs;
    int shmidA;
    sem_t *semProc1;
    if ((shmidA = shmget(IPC_PRIVATE, sizeof(sem_t)*2, (S_IRUSR|S_IWUSR))) == -1) { //desmevw 50 theseis se afti ti periptwsi char gia testing
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
    semProc1++;
    semProc2 = semProc1;
    if (sem_init(semProc2, 1, 1) == -1) {
        perror("Failed to initialize semaphore");
    } else {
        printf("Created semProc2\n");
    }
    semProc1--;
    int status;

    mkfifo(execFifo,0666);
    execfd = open(execFifo,O_RDWR | O_NONBLOCK);
    queuedProcs = qInit();
    runningProcs = qInit();
    char buf[100]; //will change later
    fd = open(namedFifo,O_RDONLY);
    while(1) { 
        if(sem_trywait(semProc1) == 0) {
            memset(buf,0,100);
            read(fd,buf,100);
            char* tok = strtok(buf,"\0");
            while(tok) {
                // printf("TOKEN %s\n",tok);
                if(strncmp(buf,"1",1) == 0) {
                    printf("Server is shutting down...\n");
                    serverClose();
                } else if(strncmp(buf,"issueJob",8) == 0) {
                    issueJob(buf+8,queuedProcs);
                    executionCheck(queuedProcs,runningProcs);
                } else if(strncmp(buf,"setConcurrency",14) == 0) {
                    int con = atoi(buf+14); 
                    if(!con) {
                        printf("Error reading argument.\n");
                    }
                    setConcurrency(con);
                    while(concurrency > running) {
                        executionCheck(queuedProcs,runningProcs);
                    }
                } else if(strncmp(buf,"stop",4) == 0) {
                    jobStop(buf+4);            
                    executionCheck(queuedProcs,runningProcs);
                } else if(strncmp(buf,"poll",4) == 0) { 
                    if(strncmp(buf+5,"running",7) == 0) {
                        pqPrint(runningProcs,0);
                    } else if(strncmp(buf+5,"queued",6) == 0) {
                        pqPrint(queuedProcs,1);
                    }
                }
                tok = strtok(NULL,"\0");
            }
        }
        pid_t test = waitpid(0,&status,WNOHANG);
        if(test && test != -1) {
            running--;
            printf("PROCESS HAS FINISHED\n");
            executionCheck(queuedProcs,runningProcs);
        }
    }
    close(fd);
    return 0;
}