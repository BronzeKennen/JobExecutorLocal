#include "../headers/pq.h"
typedef struct job {
    char* jobId; //job_XX {XX} jobNumber
    char* job; //task
    int qPos; // queue position
} jProperties;

typedef struct run {
    int pid;
    char* jobId;
} runningJobs;


int serverInit();
int serverClose();

void issueJob(char*, pQueue);
void setConcurrency(int);
void jobStop(char*);