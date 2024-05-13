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
#include <signal.h>
#include <sys/wait.h>

char* server = "./jobExecutorServer";
const char* filename = "jobExecutorServer.txt";
int serverPid = 0;
int isUp(int mode) {
    if(access(filename,F_OK) != -1 && mode == 1) {
        int fd = open(filename,O_RDONLY);
        char pid[20];
        read(fd,pid,20);
        serverPid = atoi(pid);
        close(fd);
        return serverPid;
    } 
    if(access(filename,F_OK) != -1 && mode == 2) {
        int fd = open(filename,O_RDONLY);
        char pid[20];
        read(fd,pid,20);
        char* token = strtok(pid,"\n");
        token = strtok(NULL,"\n");
        int shmid = atoi(token);
        return shmid;
    }
    if(mode == 0 && access(filename,F_OK) == -1) {
        char* args[]={server,NULL};
        execvp(server,args);
        exit(0);
    }
    return 0;
}

char* namedFifo = "/tmp/comfifo";
char* serverFifo = "/tmp/servfifo";

int main(int argc, char** argv) {

    int shmidA;


    pid_t pid;
    pid = fork();
    if(pid == -1) {
        printf("Error on fork\n");
        exit(EXIT_FAILURE);
    } else if(pid == 0 ) {
        if(strncmp(argv[1],"exit",4) == 0) exit(0);
        isUp(0);
        exit(0);
    }   

    pid_t serverPid = isUp(1);
    while(!serverPid) {
        serverPid = isUp(1);
    }
    shmidA = isUp(2);
	sem_t * semProc1 = (sem_t *) shmat(shmidA, NULL, 0); //kanw attach to shared memory tou allou process
	if (semProc1 == (void *) -1) {
		perror("Attachment."); 
		exit(2);
	}
    semProc1+=2;
    int* bytes = (int*)semProc1;
    semProc1-=2;
    semProc1++;
    sem_t * semProc2 = semProc1;
    semProc1--;
    while(*(int*)semProc2 == 0);
    //Initialize Semaphores
    
    mkfifo(namedFifo,0666);
    int fd = open(namedFifo,O_WRONLY);
    size_t total_length = 0;
    for (int i = 1; i < argc; i++) {
        total_length += strlen(argv[i]);
    }
    char *concatenated = (char*)malloc(total_length + argc);

    size_t current_pos = 0;
    for(int i = 1; i < argc; i++) {
        strcpy(concatenated+current_pos,argv[i]);
        current_pos += strlen(argv[i]);
        concatenated[current_pos++] = ' ';
    }
    if(strncmp(argv[1],"issueJob",8) == 0) {
        if(argc < 3) {
            printf("Usage: jobCommander issueJob <command>\n");
            exit(1); 
        }
        concatenated[current_pos - 1] = '\0';
        write(fd,concatenated,strlen(concatenated));
        if(*(int*)semProc1 != 1) sem_post(semProc1);
        *bytes = strlen(concatenated);
        int sd = open(serverFifo,O_RDONLY );
        char buff[*bytes+5];
        memset(buff,0,*bytes+5);
        read(sd,buff,*bytes+5);
        printf("Server responded with <%s>\n",buff);
        close(sd);
    } else if(strncmp(argv[1],"exit",4) == 0) {
        write(fd,"1",strlen("1")+1);
        
        *bytes = strlen(concatenated);
        if(*(int*)semProc1 != 1) sem_post(semProc1);
    } else if(strncmp(argv[1],"stop",4) == 0) {
        write(fd,concatenated,strlen(concatenated));
        *bytes = strlen(concatenated);
        if(*(int*)semProc1 != 1) sem_post(semProc1);
    } else if(strncmp(argv[1],"setConcurrency",14) == 0){
        if(argc < 3) {
            printf("Usage: jobCommander setConcurrency <number>\n");
            close(fd);
            return 1;
        }
        int con = atoi(argv[2]);
        if(!con) {
            printf("Usage: jobCommander setConcurrency <number>\n");
            close(fd);
            return 1;
        }
        write(fd,concatenated,strlen(concatenated));
        if(*(int*)semProc1 != 1) sem_post(semProc1);
    } else if(strncmp(argv[1],"poll",4) == 0) {
        write(fd,concatenated,strlen(concatenated));  
        if(*(int*)semProc1 != 1) sem_post(semProc1);
    } else {
        printf("Invalid argument.\n");
        exit(1);
    }
        // printf("====> %s\n",argv[2]);
    close(fd);
    return 0;
}