#include "../headers/pq.h"
typedef struct job {
    char* jobId; //job_XX {XX} jobNumber
    char* job; //task
    int qPos; // queue position
} jProperties;

typedef struct run {
    int pid; //process id
    char* jobId; // job_XX {XX} jobNumber
} runningJobs;


int serverInit(); //start server
int serverClose(); //terminate server

void issueJob(char*, pQueue); //give triplet
void setConcurrency(int);
int jobStop(char*,pQueue,int);