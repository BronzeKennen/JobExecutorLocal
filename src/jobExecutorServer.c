#include "../headers/jobExecutorServer.h"
#include "../headers/pq.h"
#include <stdlib.h>
#include <errno.h>
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
int queue = 1;
int running = 0;
int jobId = 1;

char* serverFifo = "/tmp/servfifo";
char* namedFifo = "/tmp/comfifo";
char* execFifo = "/tmp/execfifo";
const char* filename = "jobExecutorServer.txt";

int execfd,sd,fd;
int* bytes;

// Table with all processes (RUNNING/QUEUED)
// Stack with queued processes only (POP ONLY FIRST)

sem_t *semProc2;
//Horrible design but no time
pqNode pqFindJob(char* jobId, pQueue q) {
    pqNode node = q->first;
    if(!node) {
        return NULL;
    }
    jProperties* data;
    char* toCmp = strtok(jobId," ");
    while(node) {
        data = (jProperties*)node->data;
        if(strcmp(toCmp,data->jobId) == 0) return node;
        node = node->next;
    }
    if(!node) {
        return NULL;
    }
}

pqNode pqFindProc(int pid, pQueue q) {
    pqNode node = q->first;
    if(!node) {
        return NULL;
    }

    runningJobs* data;

    while(node) {
        data = (runningJobs*)node->data;
        if(pid == data->pid) return node;
        node = node->next;
    }
    if(!node) {
        return NULL;
    }
}
pqNode pqFindProc2(char* jobId,pQueue q) { 
    pqNode node = q->first;
    if(!node) {
        return NULL;
    }

    runningJobs* data;

    char* toCmp = strtok(jobId," ");
    while(node) {
        data = (runningJobs*)node->data;
        if(strcmp(toCmp,data->jobId) == 0) return node;
        node = node->next;
    }
    if(!node) {
        return NULL;
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
    fflush(stdout);
    close(fp);
    return 0;
}

void setConcurrency(int n) {
    concurrency = n;
}

void pqPrint(pQueue q,int mode) {
    if(!mode) {
        pqNode s = q->first;
        if(!s) {
            printf("No running jobs.\n");
            return;
        }
        printf("=Running=\n");
        runningJobs* data = (runningJobs*)s->data;
        char buf[(strlen(data->jobId)+9)*q->size];
        char another[strlen(data->jobId)+9];
        sprintf(buf,"%s,%d\n",data->jobId,data->pid);
        while(s) {
            s = s->next;
            if(!s) break;
            data = (runningJobs*)s->data;
            sprintf(another,"%s,%d\n",data->jobId,data->pid);
            strcat(buf,another);
        }
        int sd = open(serverFifo,O_WRONLY);
        sem_wait(semProc2);
        *bytes = sizeof(buf);
        sem_post(semProc2);
        write(sd,buf,*bytes);
        memset(buf,0,((strlen(data->jobId)+9)*q->size));
        memset(another,0,((strlen(data->jobId)+9)*q->size));

    } else{
        pqNode s = q->first;
        if(!s) {
            printf("No queued jobs.\n");
            return;
        }
        printf("=Queued=\n");
        jProperties* data = (jProperties*)s->data;
        char buf[(strlen(data->jobId)+9+strlen(data->job))*q->size];
        char another[strlen(data->jobId)+9+strlen(data->job)];
        sprintf(buf,"%s,%s,1\n",data->jobId,data->job);
        int i = 1;
        while(s) {
            s = s->next;
            if(!s) break;
            data = (jProperties*)s->data;
            sprintf(another,"%s,%s,%d\n",data->jobId,data->job,i+1);
            strcat(buf,another);
            i++;
        }
        int sd = open(serverFifo,O_WRONLY);
        sem_wait(semProc2);
        *bytes = sizeof(buf);
        sem_post(semProc2);
        write(sd,buf,*bytes);
        memset(buf,0,(strlen(data->jobId)+9+strlen(data->job))*q->size);
        memset(another,0,(strlen(data->jobId)+9+strlen(data->job)));
    }
    close(sd);
}


void issueJob(char* job,pQueue q) {

    jProperties *new = malloc(sizeof(jProperties));

    char jobString[7];
    sprintf(jobString,"job_%d",jobId);
    new->jobId = malloc(strlen(jobString));

    strncpy(new->jobId,jobString,strlen(jobString));
    new->qPos = queue++;
    jobId++;

    new->job = malloc(strlen(job));
    strncpy(new->job,job,strlen(job));

    insert(new,q);
    char toWrite[strlen(job)+strlen(new->jobId)+5];
    sprintf(toWrite,"%s,%s,%d",new->job,new->jobId,new->qPos);
    write(sd,toWrite,sizeof(toWrite));
}

void runQueueItem(pid_t* pid,char* job) {
    int shmid;
    *pid = fork();
    if(*pid == -1) {
        printf("fork\n");
        exit(EXIT_FAILURE);
    }
    if(*pid != 0) return;

    char* toTok = strdup(job);
    close(fd);
    int argCount =0;
    char* tok = strtok(toTok," ");
    while(tok) {
        argCount++;
        tok = strtok(NULL," ");
    }
    char** args =malloc((argCount + 1) * sizeof(char*));


    tok = strtok(job," ");
    int i = 0;
    while(tok) {
        args[i] = strdup(tok);
        i++;
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
        queue--;
        running++;
        insert(p,curRunning);
        runQueueItem(&pid,data->job);
        p->pid = pid;
    } 
}


int jobStop(char* jobId, pQueue q,int m) {
    pqNode toFind;
    if(m) toFind = pqFindJob(jobId,q);
    else toFind = pqFindProc2(jobId,q);
    if(!toFind) {
        return -1;
    }
    if(!m) {
        runningJobs* data = (runningJobs*)toFind->data;
        kill(data->pid,SIGKILL);
        printf("KILLED %d\n",data->pid);
    } else {
        pqRemove(toFind,q);
    }
    return 0;
}

int serverClose() {
    remove(filename);
    printf("\n--===Server has been terminated===--\n");
    exit(0);
}

int main(int argc, char** argv) {
    pQueue queuedProcs;
    pQueue runningProcs;
    int shmidA; //Shared memory id
    sem_t *semProc1;
    if ((shmidA = shmget(IPC_PRIVATE, sizeof(sem_t)*2+4, (S_IRUSR|S_IWUSR))) == -1) {
        //shared memory
        perror("Failed to create shared memory segment");
        return 1;
    } 
    serverInit(shmidA);
    if ((semProc1 = shmat(shmidA, NULL, 0)) == (void *)-1) {
        perror("Failed to attach memory segment");
        return 1;
    }
    //Initialize 2 semaphores
    if (sem_init(semProc1, 1, 0) == -1) {
        perror("Failed to initialize semaphore");
    } 
    semProc1 +=2;
    bytes = (int*)semProc1; //Commander tells server how many bytes to read
    semProc1 -=2;

    semProc1++;
    semProc2 = semProc1;
    if (sem_init(semProc2, 1, 1) == -1) {
        perror("Failed to initialize semaphore");
    }
    semProc1--;
    int status;
    if(mkfifo(serverFifo,0666) == -1) {
        if(errno != EEXIST) {
            printf("Error creating pipe.\n");
            exit(1);
        }
    }

    //Initialize queues for running/queued jobs
    queuedProcs = qInit();
    runningProcs = qInit();
    char *buf; 
    fd = open(namedFifo,O_RDONLY);
    while(1) { //trywait to still be able to check if a process has finished
        if(sem_trywait(semProc1) == 0) { 
            buf = malloc(*bytes);
            memset(buf,0,*bytes);
            if(read(fd,buf,*bytes) == -1) {
                printf("error reading bytes\n");
                continue;
            }

            char* tok = strtok(buf,"\0");
            
            while(tok) {
                if(strncmp(buf,"1",1) == 0) { //exit
                    
                    printf("Server is shutting down...\n");
                    serverClose();
                
                } else if(strncmp(buf,"issueJob",8) == 0) {
                    
                    sd = open(serverFifo,O_WRONLY); //send back to commander the struct
                    issueJob(buf+8,queuedProcs); //add to queued jobs
                    close(sd);
                    executionCheck(queuedProcs,runningProcs);

                } else if(strncmp(buf,"setConcurrency",14) == 0) {
                    int con = atoi(buf+14); 
                
                    if(!con)
                        printf("Error reading argument.\n");
                
                    setConcurrency(con);
                    //check if there is a process to run enough to fill max amount of 
                    //concurrent processes running.
                    while(concurrency > running) 
                        executionCheck(queuedProcs,runningProcs);
                    
                } else if(strncmp(buf,"stop",4) == 0) {
                    
                    if(pqFindJob(buf+5,queuedProcs) == NULL) {
                        if(pqFindProc2(buf+5,runningProcs) == NULL) {
                            printf("id %s not found.\n",(buf+5));
                        } else {
                            jobStop(buf+5,runningProcs,0);
                            printf("%s stopped succesfully\n",(buf+5));
                        }
                    } else {
                        jobStop(buf+5,queuedProcs,1);
                        printf("%s stopped succesfully\n",(buf+5));
                    }
                
                } else if(strncmp(buf,"poll",4) == 0) { 
                
                    if(strncmp(buf+5,"running",7) == 0) pqPrint(runningProcs,0);
                    else if(strncmp(buf+5,"queued",6) == 0) pqPrint(queuedProcs,1);
                
                } else {
                    printf("An error occured\n");
                }
                tok = strtok(NULL,"\0");
            }
        }
        pid_t test = waitpid(0,&status,WNOHANG); //check if a process has finished
        if(test && test != -1) { 
            printf("PROC FINISHED\n");
            running--; //make space for another process to run
            pqNode node = pqFindProc(test,runningProcs);
            // runningJobs* data = (runningJobs*)node->data;
            pqRemove(node,runningProcs); //remove from the running processes
            executionCheck(queuedProcs,runningProcs); //check if theres is a process to run
        }
    }
    close(fd);
    return 0;
}