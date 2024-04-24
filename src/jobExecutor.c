#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../headers/jobCommander.h"
#include "../headers/jobExecutorServer.h"
int concurrency = 1;

int main(int argc, char** argv) {

    char string[40];
    char *token;

    int s = serverInit();
    isUp();
    while(1) {
        fgets(string,39,stdin);
        token = strtok(string," ");
        if(strcmp(token,"jobCommander") != 0) {
            printf("Invalid Command.\n");
            continue;
        }
        token = strtok(NULL," ");
        if(strncmp(token,"exit",4) == 0) {
            printf("Program is exiting with code 0.\n");
            break;
        } else if(strncmp(token,"issueJob",8) == 0) {
            char arg[30];
            token = strtok(NULL," ");
            if(!token) {
                printf("Invalid Argument.\n");
                continue;
            }

            strcat(arg,token);
            while(1) { 
                token = strtok(NULL," ");
                if(!token) {
                    break;
                }
                strcat(arg," ");
                strcat(arg,token);
            }
            strcat(arg,"\0");
            printf("%s",arg);
        } else if(strncmp(token,"stop",4) == 0) {
            token = strtok(NULL," ");
            jobStop(atoi(token));
        } else if(strncmp(token,"poll",4) == 0) {

        } else if(strncmp(token,"setConcurrency",15) == 0) {
            token = strtok(NULL," ");
            concurrency = atoi(token);
            printf("==-- Concurrency has been set to %d. --==\n",concurrency);
        }

    } 
    serverClose();
    return 0;
}