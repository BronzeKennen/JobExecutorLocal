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
    //    printf("%p,%p\n",q->first,q->first->next);
       q->first = q->first->next;
    //    printf("AFTER %p,%p\n",q->first,q->first->next);
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
    while (s) {
        if(s == node) {
            f->next = s->next;
            q->size--;
            return;
        }
        f = f->next;
        s = s->next;
    }
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
        q->first->next = malloc(sizeof(*q->first->next));
        q->first->next->data = new;
        q->last = q->first->next;
        //second element is last on list, and next one after first
    } else  {    
        q->last->next = malloc(sizeof(*q->last->next));
        q->last->next->data = new;
        q->last = q->last->next;
        //whichever element enters is now last, therefore next of the previous last
    }
    q->size++;
}