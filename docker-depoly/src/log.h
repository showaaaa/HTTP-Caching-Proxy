#include <mutex>
#include <ctime>
#include <fstream>
#include <iostream>

using namespace std;

class Log{
    const string fileLocation = "/var/log/erss/proxy.log";
    //const string fileLocation = "/home/xw202/hw2/proxy2.log";
    
    std::ofstream ofs;
    mutex lock;

    public:
        Log(){
            ofs.open(fileLocation, std::ofstream::out | std::ofstream::app);
            if(ofs.is_open()){
                cout << "logger file open\n";
            }else{
                throw iostream::failure("Unable to open the log file");
            }
        }

        void writeString(string &content){
            if(ofs.is_open() && lock.try_lock()){
                cout << "writing\n";
                ofs << content;
                ofs.flush();
                lock.unlock();
            }
        }

        void closeFile(){
            ofs.close();
            
        }


};
