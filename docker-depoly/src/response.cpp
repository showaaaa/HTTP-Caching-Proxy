#include "response.h"

void Response::parseResponseLine(){
    size_t pos = first.find("\r\n");
    responseLine = first.substr(0, pos);
}

void Response::parseField(){
    size_t pos = first.find("no-cache");
    if(pos != string::npos){
        nocache = true;
    }

    pos =first.find("ETag: ");
    if(pos != string::npos){
        size_t end = first.find("\r\n", pos);
        ETag = first.substr(pos + 6, end - pos - 6);
    }

    pos = first.find("Last-Modified: ");
    if(pos != string::npos){
        size_t end = first.find("\r\n", pos);
        Lastmodified = first.substr(pos + 15, end - pos - 15);
    }

    pos = first.find("max-age=");
    if(pos != string::npos){
        string age = first.substr(pos + 8);
        maxage = atoi(age.c_str());
    }

    pos = first.find("must-revalidate");
    if(pos != string::npos){
        mustrevalidate = true;
    }

    pos = first.find("Expires: ");
    if(pos != string::npos){
        size_t end = first.find(" GMT", pos);
        expires = first.substr(pos + 9, end - pos - 9); 
    }

    pos = first.find("no-store");
    if(pos != string::npos){
        nostore = true;
    }

    pos = first.find("private");
    if(pos != string::npos){
        privat = true;
    }
}

bool Response::check_malformed(){
    if(first.find("\r\n\r\n") == string::npos){
        return false;
    }
    return true;
}

//if true to store in cache
bool Response::checkToStore(){
    if(nostore || privat){
        return false;
    }
    return true;
}

/*
int Response::checkStatus(Request *req){
    if(nocache || maxage==0){
        return REQ_NOCACHE;  //revalidation
    }
    if(maxage==0){
        return RES_MAXAGE_ZERO;  //revalidation
    }
    if(maxage > 0){
        size_t tmp = req->maxage < maxage ? req->maxage : maxage;

    }

}
*/