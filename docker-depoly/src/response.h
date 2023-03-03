#include <stdio.h>
#include <string.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include "request.h"

using namespace std;

class Response{
    public:
        string first;   //first recv from server
        string rest;    //last while loop recv from server
        string responseLine;
        bool nocache;
        string ETag;
        string Lastmodified;
        time_t receivedtime;  //response receive time
        int maxage;
        bool mustrevalidate;
        string expires;
        bool nostore;
        bool privat;

    
    //constrructor

    Response(string in): first(in), rest(""), responseLine(""), nocache(false), ETag(""), Lastmodified(""), 
    receivedtime(time(0)), maxage(-1), mustrevalidate(false), expires(""), nostore(false), privat(false){
        // parse_method();
        // parse_uri();
        // parse_host_path_port();
        //parseResponseLine();
        //parseField();
        parseResponseLine();
        parseField();
    }
    Response(string in, string later): first(in), rest(later), responseLine(""), nocache(false), ETag(""), Lastmodified(""), 
    receivedtime(time(0)), maxage(-1), mustrevalidate(false), expires(""), nostore(false), privat(false){
        // parse_method();
        // parse_uri();
        // parse_host_path_port();
        //parseResponseLine();
        //parseField();
        parseResponseLine();
        parseField();
    }
    void parseResponseLine();
    void parseField();

    bool check_malformed();

    // void parse_method();
    // void parse_uri();
    // void parse_host_path_port();
    // void buildRequest();

    bool checkToStore();
    // vector<char> buildRequest();
    int checkStatus(Request *req);
    
};