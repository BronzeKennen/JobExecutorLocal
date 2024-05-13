#include <stdlib.h>
#include <stdio.h>
#include "../headers/pq.h"
pQueue qInit() {
    pQueue q = malloc(sizeof(*q));
    q->size = 0;
    q->first = NULL;
    q->last = NULL;
    return q;
}

pqNode pqGetFirst(pQueue q) {
    return q->first;
}

void pqPopFirst(pQueue q) {
    if(q->first == NULL) {
        return;
    }
    if(q->first->next) {
       q->first = q->first->next;
    } else {
        free(q->first);
        q->first = NULL;
    }
    q->size--;
}


void pqRemove(pqNode node, pQueue q) {
    pqNode f = q->first;
    if(!f) return;
    if(node == f) {
       pqPopFirst(q); 
       return;
    } 
    pqNode s = q->first->next;
    if(!s) return;
    for(int i = 0; i < q->size-1; i++) {
        if(s == node) {
            f->next = s->next;
        }
        f = f->next;
        s = s->next;
    }
    q->size--;
}
void insert(void* new, pQueue q) {

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