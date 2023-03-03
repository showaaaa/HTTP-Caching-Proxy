#include <stdio.h>
#include <string.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class Request{
    public:
        string input;
        string uri;
        string method;
        string host;
        string path;
        string port;
        string result;
        string requestLine;
        int maxage;
        int minfresh;
        int maxstale;
    
    //constrructor

    Request(string in): input(in), maxage(2147483647), minfresh(0), maxstale(0){
        // parse_method();
        // parse_uri();
        // parse_host_path_port();
        // parse_method();
        // parse_uri();
        // parse_host_path_port();
        // parse_firstline();
    }

    bool check_malformed_build();

    void parse_method();
    void parse_uri();
    void parse_host_path_port();
    void parse_firstline();
    void parse_time();
    void buildRequest();


    // vector<char> buildRequest();
    
};