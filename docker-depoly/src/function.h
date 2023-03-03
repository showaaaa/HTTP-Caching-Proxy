#include <memory.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <semaphore.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <ostream>
#include <assert.h>
#include <signal.h>

using namespace std;

void P(sem_t* sem);

void V(sem_t* sem);