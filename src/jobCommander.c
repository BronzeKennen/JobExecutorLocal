#include "../headers/jobExecutorServer.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>



int isUp() {
    int fd = open("jobExecutorServer.txt",O_RDONLY);
    if(fd) {
        char pid[10];
        read(fd,pid,10);
        printf("%s\n",pid);
        close(fd);
        return 0;
    } else {
        return 1;
    }
}

char* namedFifo = "bin/comfifo";

int main(int argc, char** argv) {

    pid_t pid;
    pid = fork();
    if(pid == -1) {
        printf("Error on fork\n");
        exit(EXIT_FAILURE);
    } else if(pid == 0 ) {
        exit(0);
    }

    mkfifo(namedFifo,0666);

    int fd;

    fd = open(namedFifo,O_WRONLY | O_NONBLOCK);

    if(argc <=1) {
        printf("Usage: jobCommander  <command> \n");
        exit(1);
    }
    int err = isUp();
    printf("server = %d\n",err);
    if(strncmp(argv[1],"issueJob",8) == 0) {
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
        concatenated[current_pos - 1] = '\0';
        write(fd,concatenated,strlen(concatenated));
    } else if(strncmp(argv[1],"exit",4) == 0) {
        write(fd,"1",strlen("1")+1);
    } else if(strncmp(argv[1],"stop",4) == 0) {
        printf("Attempting to stop process _\n");
    } else {
        perror("Invalid argument.\n");
        exit(1);
    }
    close(fd);
    return 0;
}