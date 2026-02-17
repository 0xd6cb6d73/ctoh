// https://github.com/trustedsec/CS-Situational-Awareness-BOF/blob/1c93d1b66ec9b1c14a0679b3ecc592a950c82537/src/common/queue.c#L9
typedef struct _queue{\
    void* head;
    void* tail;
    void (*push)(struct _queue *, void *);
    void * (*pop)(struct _queue *);
    void (*free)(struct _queue *);
}queue, *Pqueue;