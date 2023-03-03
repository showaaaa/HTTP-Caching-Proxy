#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

using namespace std;

map<std::string, int> time_map{
    {"Jan", 1},
    {"Feb", 2},
    {"Mar", 3},
    {"Apr", 4},
    {"May", 5},
    {"Jun", 6},
    {"Jul", 7},
    {"Aug", 8},
    {"Sep", 9},
    {"Oct", 10},
    {"Nov", 11},
    {"Dec", 12},
    {"Mon", 1},
    {"Tue", 2},
    {"Wed", 3},
    {"Thu", 4},
    {"Fri", 5},
    {"Sat", 6},
    {"Sun", 0}
};

void parseHttpdate(string t, struct tm *res){
    
    res->tm_mday = atoi(t.substr(5).c_str());
    //res.tm_mon = time_map.getMap(t.substr(8, 3).c_str()) - 1;
    res->tm_mon = time_map[t.substr(8, 3).c_str()] - 1;
    res->tm_year = atoi(t.substr(12).c_str()) - 1900;
    res->tm_hour = atoi(t.substr(17).c_str());
    res->tm_min = atoi(t.substr(20).c_str());
    res->tm_sec = atoi(t.substr(23).c_str());
    //res.tm_wday = time_map.getMap(t.substr(0, 3).c_str());
    res->tm_wday = time_map[t.substr(0, 3).c_str()];
    res->tm_isdst = 0;

    cout << time_map[t.substr(0, 3).c_str()] << endl;
    cout << "parse finished\n";
    //return res;
}

//convert current time to utc time
string utctime(){
    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    string res = asctime(gmtm);
    return res;
}

string utcconvert(time_t pre){
    tm *gmtm = gmtime(&pre);
    string lastpart = asctime(gmtm);
    return lastpart;
}

//std::string result = asctime(gmtime(&inputTime));