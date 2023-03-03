#include <stdio.h>
#include <string.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include "response.h"
#include <unordered_map>
#include "log.h"
#include "sbuf.h"
#include "timefunction.h"
using namespace std;

class Node{
    public:
        string requestLine;
        Response *res;
        Node * next;
        Node * prev;

        Node():requestLine(""), res(nullptr), next(nullptr), prev(nullptr){}
        Node(string rl, Response *re): requestLine(rl), res(re), next(nullptr), prev(nullptr){}

};

class Cache{
    public:
        Node* head;
        Node* tail;
        unordered_map<string, Node*> map;
        
        int capacity;
        int size;

        pthread_rwlock_t rwlock;

        Cache(int c): capacity(c), size(0){
            head = new Node();
            tail = new Node();
            head->next = tail;
            tail->prev = head;
            pthread_rwlock_init(&rwlock, NULL);
        }

        Response* get(string key){
            pthread_rwlock_rdlock(&rwlock);
            if(!map.count(key)){
                pthread_rwlock_unlock(&rwlock);
                return nullptr;
            }
            pthread_rwlock_unlock(&rwlock);
            cout << "unlock get successful\n";
            return map[key]->res;
            
        }

        void put(string key, Response *resp, Log *log, Client_Info &info){
            cout << "to lock put\n";
            // string test0 = "to lock put\n";
            // log->writeString(test0);
            pthread_rwlock_wrlock(&rwlock);
            cout << "locked put\n";
            Response * instore = new Response(resp->first, resp->rest);
            if(!map.count(key)){
                Node * node = new Node(key, instore);
                map[key] = node;
                addToHead(node);
                size++;
                if(size > capacity){
                    Node* removed = removeTail();
                    map.erase(removed->requestLine);
                    delete removed->res;
                    delete removed;
                    size--;
                }
            }else{
                Node * node = map[key];
                node->res = instore;
                moveToHead(node);
            }

            // test0 = "Note log cache if else\n";
            // log->writeString(test0);

            // string test1 = resp->nocache? "true" : "false";
            // test0 = "resp's nocahe: " + test1;
            // log->writeString(test0);
            if(!resp->nocache && resp->maxage == -1 && resp->expires == ""){
                string fivelog = info.getUUID() + ": " +  "cached, no expires time\n";
                log->writeString(fivelog);
            }
            else if(resp->nocache || resp->maxage == 0){
                string fivelog = info.getUUID() + ": " +  "cached, but requires re-validation";
                log->writeString(fivelog);
            }else{
                if(resp->maxage > 0){
                    time_t expir = resp->maxage + resp->receivedtime;
                    string lastpart = utcconvert(expir);
                    string fivelog = info.getUUID() + ": cached, " +  "expires at " + lastpart;
                    log->writeString(fivelog);
                }else if(resp->expires != ""){
                    struct tm res;
                    parseHttpdate(resp->expires, &res);
                    string lastpart = asctime(&res);
                    string fivelog = info.getUUID() + ": cached, " +  "expires at " + lastpart;
                    log->writeString(fivelog);
                }
                
            }
            
            pthread_rwlock_unlock(&rwlock);
        }

        void addToHead(Node* node) {
            node->prev = head;
            node->next = head->next;
            head->next->prev = node;
            head->next = node;

        }

        void removeNode(Node * node){
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        Node * removeTail(){
            Node* node = tail->prev;
            removeNode(node);
            return node;
        }

        void moveToHead(Node* node){
            removeNode(node);
            addToHead(node);
        }


};

enum CachePolicy {
    RES_NOCACHE_MAXAGE_ZERO,
    RES_,
    

};
// pthread_rwlock_rdlock(&cache.cacheobjs[i].rwlock);
//         if((cache.cacheobjs[i].isEmpty==0) && (strcmp(url,cache.cacheobjs[i].cache_url)==0)) break;
//         pthread_rwlock_unlock(&cache.cacheobjs[i].rwlock);

// pthread_rwlock_wrlock(&cache.cacheobjs[i].rwlock);
//         if(cache.cacheobjs[i].isEmpty==0 && i!=index){
//             cache.cacheobjs[i].LRU--;
//         }
//         //writeAfter(i);
//         pthread_rwlock_unlock(&cache.cacheobjs[i].rwlock);