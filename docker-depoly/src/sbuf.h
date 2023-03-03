#include <vector>
#include "client_info.h"
#include "function.h"
using namespace std;


class Sbuf{
    public:
        vector<Client_Info>buf; /* Buffer array */ 
        int n;                  /* Maximum number of slots */
        int front;              /* buf[(front+1)%n] is first item */
        int rear;               /* buf[rear%n] is last item */
        sem_t mutex;            /* Protects accesses to buf */
        sem_t slots;            /* Counts available slots */
        sem_t items;            /* Counts available items */

        Sbuf(int num);
        void subf_insert(Client_Info info);
        Client_Info subf_remove();
};