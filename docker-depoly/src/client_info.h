#include <vector>
#include <string>
using namespace std;

class Client_Info{
    private:
        string UUID;
        int fd;
        string ip;
        //string request;
        
        
    public:
        Client_Info(string id, int cfd, string cip){
            UUID = id;
            fd = cfd;
            ip = cip;
        }
        Client_Info(){

        }
        void setUUID(string id){
            UUID = id;
        }
        void setFD(int cfd){
            fd = cfd;
        }
        void setIP(string cip){
            UUID = cip;
        }
        // void setRequest(string req){
        //     request = req;
        // }
        string getUUID(){
            return UUID;
        }
        int getFD(){
            return fd;
        }
        string getIP(){
            return ip;
        } 
        // string getRequest(){
        //     return request;
        // }    
};