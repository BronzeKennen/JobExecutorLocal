int serverInit();
int serverClose();

void issueJob(char*);
void setConcurrency(int);
void jobStop(int);
typedef struct job {
    char* jobId; //job_XX {XX} jobNumber
    char* job; //task
    int qPos; // queue position
} jProperties;