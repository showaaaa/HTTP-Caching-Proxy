#include "sbuf.h"
Sbuf::Sbuf(int num){
    this->n = num;
    this->buf = vector<Client_Info>(num);
    this->front = 0;
    this->rear     = 0;
    sem_init(&this->mutex, 0, 1);
    sem_init(&this->slots, 0, num);
    sem_init(&this->items, 0, 0);
}

void Sbuf::subf_insert(Client_Info info){
    P(&this->slots);
    P(&this->mutex);

    this->buf[(++this->rear)%(this->n)] = info;

    V(&this->mutex);                          
    V(&this->items);
}

Client_Info Sbuf::subf_remove(){
    Client_Info info;

    P(&this->items);                          
    P(&this->mutex);                          
    info = this->buf[(++this->front)%(this->n)];  
    V(&this->mutex);                          
    V(&this->slots);

    return info;
}