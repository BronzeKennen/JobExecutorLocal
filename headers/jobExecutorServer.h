typedef struct job {
    char* jobId; //job_XX {XX} jobNumber
    char* job; //task
    int qPos; // queue position
} jProperties;

typedef struct priorityQueueNode {
    jProperties *data;
    struct priorityQueueNode *next;
} *pqNode;

typedef struct priorityQueue { 
    pqNode first;
    pqNode last;
    int size;
} *pQueue;

int serverInit();
int serverClose();

void issueJob(char*);
void setConcurrency(int);
void jobStop(char*);