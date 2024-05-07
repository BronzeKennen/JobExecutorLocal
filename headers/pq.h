#pragma once
typedef struct priorityQueueNode {
    void *data;
    struct priorityQueueNode *next;
} *pqNode;

typedef struct priorityQueue { 
    pqNode first;
    pqNode last;
    int size;
} *pQueue;

pqNode pqGetFirst(pQueue);
pQueue qInit();
void pqPopFirst(pQueue);
void pqRemove(pqNode, pQueue);