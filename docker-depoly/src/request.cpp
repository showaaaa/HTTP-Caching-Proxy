#include "request.h"

void Request::parse_method(){
    size_t after_method = input.find(" ");
    method = input.substr(0, after_method);
}

void Request::parse_uri(){
    size_t uri_start = input.find(" ") + 1;
    cout << "uristart:" << uri_start << endl;
    size_t uri_end = input.find(" ", uri_start);
    cout << "uriend:" << uri_end << endl;
    uri = input.substr(uri_start, uri_end - uri_start);
}

void Request::parse_host_path_port(){
    port = "80";
    path = "";
    size_t host_start = uri.find("http://");
    if(host_start == string::npos){
        host_start = 0;
    }else{
        host_start += 7;
    }
    size_t host_end = uri.find(":", host_start);
    //have port
    if(host_end != string::npos){
        host = uri.substr(host_start, host_end - host_start);
        size_t path_start = uri.find("/", host_end + 1);
        //have path
        if(path_start != string::npos){
            port = uri.substr(host_end + 1, path_start - host_end - 1);
            path = uri.substr(path_start);
        //no path
        }else{
            port = uri.substr(host_end + 1);
        }
    //no port
    }else{
        host_end = uri.find("/", host_start);
        //have path
        if(host_end != string::npos){
            host = uri.substr(host_start, host_end - host_start);
            path = uri.substr(host_end);
        //no path
        }else{
            host = uri.substr(host_start);
        }
    }

}

void Request::parse_firstline(){
    size_t pos = input.find("\r\n");
    requestLine = input.substr(0, pos);
}


bool Request::check_malformed_build(){
    try{
        parse_method();
        parse_uri();
        parse_host_path_port();
        parse_firstline();
        parse_time();
        size_t empty_line = input.find("\r\n\r\n");
        if(empty_line == string::npos){
            cout << "npos" << endl;
            return false;
        }
        buildRequest();
        return true;
    }
    catch(exception &e){
        cout << "parse not valid!" << endl;
        return false;
    }
}

//to be tested
// vector<char> Request::buildRequest(){
//     size_t second_line = input.find("\r\n") + 2;
//     string result;
//     result = method + " " + path + " " + "HTTP/1.1\r\n" + input.substr(second_line);
//     vector<char> re(result.begin(), result.end());
//     return re;
// }

void Request::buildRequest(){
    size_t second_line = input.find("\r\n") + 2;
    result = method + " " + path + " " + "HTTP/1.1\r\n" + input.substr(second_line);
    cout << "build finished\n";
    //vector<char> re(result.begin(), result.end());
    //return re;
}

void Request::parse_time(){
    size_t pos = input.find("max-age=");
    if(pos != string::npos){
        string age = input.substr(pos + 8);
        maxage = atoi(age.c_str());
    }

    pos = input.find("min-fresh=");
    if(pos != string::npos){
        string age2 = input.substr(pos + 10);
        minfresh = atoi(age2.c_str());
    }

    pos = input.find("max-stale=");
    if(pos != string::npos){
        string age3 = input.substr(pos + 10);
        maxstale = atoi(age3.c_str());
    }
}

