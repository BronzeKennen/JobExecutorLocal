#include <stdlib.h>
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
    if(node == f) pqPopFirst(q);
    pqNode s = q->first->next;
    for(int i = 0; i < q->size-1; i++) {
        if(s == node) {
            f->next = s->next;
        }
        f = f->next;
        s = s->next;
    }
    q->size--;
}