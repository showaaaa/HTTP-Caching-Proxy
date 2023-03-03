#include "function.h"
#include <stdio.h>
// #include "sbuf.h"
//#include "request.h"
//#include "response.h"
#include "socket.h"
#include "cache.h"
// #include "timefunction.h"
//#include "log.h"
#define NTHREADS 200
#define SBUFSIZE 200
//#define MAXLINE 8192
#define MAXLINE 65536
#define CACHESIZE 50

Sbuf sbuf(SBUFSIZE);
Log log;
Cache cache(CACHESIZE);

//return true if use in cache
bool sendRevalidate(Response* cacheres, Request &request, int clientfd, Client_Info &info){
    if(cacheres->ETag == "" && cacheres->Lastmodified == ""){
        return true; //use in cache
    }
    string newreq = request.input;
    if(cacheres->ETag != ""){
        string add_etag = "If-None-Match: " + cacheres->ETag + "\r\n";
        newreq = newreq.insert(newreq.length() - 2, add_etag);
    }

    if(cacheres->Lastmodified != ""){
        string add_lm = "If-Modified-Since: " + cacheres->Lastmodified + "\r\n";
        newreq = newreq.insert(newreq.length() - 2, add_lm);
    }

    //send to server
    const char * host = request.host.c_str();
    const char * port = request.port.c_str();
    int serverfd = open_clientfd(host, port);
    if(serverfd < 0){
        printf("proxy to server Connection failed\n");
        //clienterror(fd, method, "400", "Bad Request",
                    //"Connection failed");
        return false;
    }
    vector<char> newreqvec(newreq.begin(), newreq.end());
    int len = send(serverfd, newreqvec.data(), MAXLINE, 0);

    string threelog = info.getUUID() + ": " +  "Requesting " + request.requestLine + " from " + request.host + "\n";
    log.writeString(threelog);

    //receive from server
    //vector<char> buffer(MAXLINE);
    vector<char> firstBuffer;
    //vector<char> restBuffer;
    len = recv(serverfd, newreqvec.data(), newreqvec.size(), 0);
    if(len <= 0){
        cout << "failed first receive from server on data\n";
        close(serverfd);
        return false;
    }
    
    string validateres = string(newreqvec.begin(), newreqvec.begin() + len);
    
    if(validateres.find("200 OK") == string::npos){
        string fourlog = info.getUUID() + ": " +  "Received " + "\"HTTP/1.1 200 OK\"" + " from " + request.host + "\n";
        log.writeString(fourlog);
        close(serverfd);
        return true; //use in cache
    }
    Response rp(validateres);
    if(!rp.check_malformed()){
        //to do
        cout << "to do send 502 error code" << endl;
        send(clientfd, "HTTP/1.1 502 Bad Gateway\r\n\r\n", 28, 0);
        string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 502 Bad Gateway\"\n";
        log.writeString(sixlog);
        close(serverfd);
        return false;
    }
    //send first part
    send(clientfd, newreqvec.data(), len, 0);
    //receive all from server and send all to client
    vector<char> newbuffer;
    while((len = recv(serverfd, newreqvec.data(), newreqvec.size(), 0)) > 0){
        newbuffer.insert(newbuffer.end(), newreqvec.begin(), newreqvec.begin() + len);
        send(clientfd, newreqvec.data(), len, 0);
    }
    string restResponse = string(newbuffer.begin(), newbuffer.end());
    //cout << endl << restResponse << endl;
    //cout << "end of response\n";
    rp.rest = restResponse;
    
    string fourlog = info.getUUID() + ": " +  "Received \"" + rp.responseLine + "\" from " + request.host + "\n";
    log.writeString(fourlog);
    string sixlog = info.getUUID() + ": " +  "Responding \"" + rp.responseLine + "\"\n";
    log.writeString(sixlog);
    //check and  to store in cache
    if(rp.checkToStore()){
        cache.put(request.requestLine, &rp, &log, info);
    }else{
        string fivelog = info.getUUID() + ": " +  "not cacheable because response contain \"nostore\" or \"private\"";
        log.writeString(fivelog);
    }
    return false;
}

//return true if use in cache
bool checktime(Response* cacheres, Request &request, int clientfd, Client_Info &info){
    if(cacheres->maxage == -1 && cacheres->expires == ""){ //use in cache if both are not exist
        return true;
    }
    size_t finalage;
    if(cacheres->maxage > 0){ //response maxage>0
        
        if(cacheres->mustrevalidate){   //has must-revalidate flag
            if(request.maxage != -1){ //request has maxage
                finalage = min(request.maxage, cacheres->maxage) - request.minfresh;
            }else{ //request no maxage
                finalage = cacheres->maxage - request.minfresh;
            }
        }else{  //no must-revalidate flag
            if(request.maxage != -1){ //request has maxage
                finalage = min(request.maxage, cacheres->maxage) - request.minfresh + request.maxstale;
            }else{
                finalage = cacheres->maxage - request.minfresh + request.maxstale;
            }
        }
        double dif = difftime(time(0), cacheres->receivedtime);
        //below
        if(dif <= finalage){
            return true; //use cache
        }else{
            //contact server
            //get maxage expire time
            time_t exp3 = cacheres->receivedtime + finalage;
            tm *gmtm3 = gmtime(&exp3);
            string res3 = asctime(gmtm3);
            string twolog = info.getUUID() + ": " +  "in cache, but expired at " + res3;
            log.writeString(twolog);
            const char * host = request.host.c_str();
            const char * port = request.port.c_str();
            int serverfd = open_clientfd(host, port);
            if(serverfd < 0){
                printf("proxy to server Connection failed\n");
                //clienterror(fd, method, "400", "Bad Request",
                            //"Connection failed");
                            
                return false;
            }
            vector<char> newreqvec(request.input.begin(), request.input.end());
            int len = send(serverfd, newreqvec.data(), MAXLINE, 0);

            string threelog = info.getUUID() + ": " +  "Requesting " + request.requestLine + " from " + request.host + "\n";
            log.writeString(threelog);

            //receive from server
            // vector<char> buffer(MAXLINE);
            // vector<char> firstBuffer(MAXLINE);
            // vector<char> restBuffer;
            len = recv(serverfd, newreqvec.data(), newreqvec.size(), 0);
            

            //new add
            string validateres = string(newreqvec.begin(), newreqvec.begin() + len);
    
            Response rp(validateres);
            if(!rp.check_malformed()){
                //to do
                cout << "to do send 502 error code" << endl;
                send(clientfd, "HTTP/1.1 502 Bad Gateway\r\n\r\n", 28, 0);
                string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 502 Bad Gateway\"\n";
                log.writeString(sixlog);
                close(serverfd);
                return false;
            }
            //send first part
            send(clientfd, newreqvec.data(), len, 0);
            //receive all from server and send all to client
            vector<char> newbuffer;
            while((len = recv(serverfd, newreqvec.data(), newreqvec.size(), 0)) != 0){
                newbuffer.insert(newbuffer.end(), newreqvec.begin(), newreqvec.begin() + len);
                send(clientfd, newreqvec.data(), len, 0);
            }
            string restResponse = string(newbuffer.begin(), newbuffer.end());
            //cout << endl << restResponse << endl;
            cout << "end of response\n";
            rp.rest = restResponse;
            
            string fourlog = info.getUUID() + ": " +  "Received " + rp.responseLine + " from " + request.host + "\n";
            log.writeString(fourlog);
            string sixlog = info.getUUID() + ": " +  "Responding \"" + rp.responseLine + "\"\n";
            log.writeString(sixlog);
            //check and  to store in cache
            if(rp.checkToStore()){
                cache.put(request.requestLine, &rp, &log, info);
            }else{
                string fivelog = info.getUUID() + ": " +  "not cacheable because response contain \"nostore\" or \"private\"\n";
                log.writeString(fivelog);
            }
            close(serverfd);
            return false;
        }
        
    }
    if(cacheres->expires != ""){ // no maxage have expires
        struct tm tm1;
        parseHttpdate(cacheres->expires, &tm1);
        time_t expi = mktime(&tm1);
        //get current time and conver to utc time
        time_t now = time(0);
        tm *gmtm = gmtime(&now);
        now = mktime(gmtm);

        double dif1 = difftime(expi, now);
        if(dif1 >=0){
            return true; //use cache
        }else{
            //contact server
            //get expire time
            string res4 = asctime(&tm1);
            string twolog = info.getUUID() + ": " +  "in cache, but expired at " + res4;
            log.writeString(twolog);
            const char * host = request.host.c_str();
            const char * port = request.port.c_str();
            int serverfd = open_clientfd(host, port);
            if(serverfd < 0){
                printf("proxy to server Connection failed\n");
                //clienterror(fd, method, "400", "Bad Request",
                            //"Connection failed");
                return false;
            }
            vector<char> newreqvec(request.input.begin(), request.input.end());
            int len = send(serverfd, newreqvec.data(), MAXLINE, 0);

            string threelog = info.getUUID() + ": " +  "Requesting " + request.requestLine + " from " + request.host + "\n";
            log.writeString(threelog);

            //receive from server
            
            vector<char> firstBuffer;
            len = recv(serverfd, newreqvec.data(), newreqvec.size(), 0);
            if(len <= 0){
                cout << "failed first receive from server on data\n";
                return false;
            }

            //new add
            string validateres = string(newreqvec.begin(), newreqvec.begin() + len);
    
            Response rp(validateres);
            if(!rp.check_malformed()){
                //to do
                cout << "to do send 502 error code" << endl;
                send(clientfd, "HTTP/1.1 502 Bad Gateway\r\n\r\n", 28, 0);
                string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 502 Bad Gateway\"\n";
                log.writeString(sixlog);
                close(serverfd);
                return false;
            }
            //send first part
            send(clientfd, newreqvec.data(), len, 0);
            vector<char> newbuffer;
            //receive all from server and send all to client
            while((len = recv(serverfd, newreqvec.data(), newreqvec.size(), 0)) != 0){
                newbuffer.insert(newbuffer.end(), newreqvec.begin(), newreqvec.begin() + len);
                send(clientfd, newreqvec.data(), len, 0);
            }
            string restResponse = string(newbuffer.begin(), newbuffer.end());
            //cout << endl << restResponse << endl;
            cout << "end of response\n";
            rp.rest = restResponse;
            
            string fourlog = info.getUUID() + ": " +  "Received " + rp.responseLine + " from " + request.host + "\n";
            log.writeString(fourlog);
            string sixlog = info.getUUID() + ": " +  "Responding \"" + rp.responseLine + "\"\n";
            log.writeString(sixlog);
            //check and  to store in cache
            if(rp.checkToStore()){
                cache.put(request.requestLine, &rp, &log, info);
            }else{
                string fivelog = info.getUUID() + ": " +  "not cacheable because response contain \"nostore\" or \"private\"\n";
                log.writeString(fivelog);
            }
            close(serverfd);
            return false;
        }
}
return true;
}
void handleGet(Request &request, int clientfd, Client_Info &info){

    Response* cacheres = cache.get(request.requestLine);
    if(cacheres == nullptr){
        string test0 = "Note: enter handleGet, cacheres nullptr\n";
        log.writeString(test0);
    }else{
        string test0 = "Note: found in cache\n";
        log.writeString(test0);
    }
    
    //found in cache
    if(cacheres != nullptr){
        if(cacheres->nocache || cacheres->maxage == 0){  //response has no-cache flag or max-age=0 (need revalidation)
            //build request and receive response from server
            
            string test0 = "Note: response has no-cache flag or max-age=0\n";
            log.writeString(test0);

            // string test1 = cacheres->nocache? "Note true": "false";
            // log.writeString(test1);

            // test1 = cacheres->maxage == 0? "0": "not 0\n";
            // log.writeString(test1);

            string twolog = info.getUUID() + ": " +  "in cache, requires validation\n";
            log.writeString(twolog);
            if(sendRevalidate(cacheres, request, clientfd, info)){ //use cache
                string tosend = cacheres->first + cacheres->rest;
                vector<char> tosendvec(tosend.begin(), tosend.end());
                send(clientfd, tosendvec.data(), MAXLINE, 0);
                string sixlog = info.getUUID() + ": " +  "Responding \"" + cacheres->responseLine + "\"\n";
                log.writeString(sixlog);
            }
            return;
        }else{  //no nocache flag && maxage!=0 to checktime
            if(checktime(cacheres, request, clientfd, info)){ // use in cache
                string twolog = info.getUUID() + ": " +  "in cache, valid\n";
                log.writeString(twolog);
                string tosend = cacheres->first + cacheres->rest;
                vector<char> tosendvec(tosend.begin(), tosend.end());
                send(clientfd, tosendvec.data(), MAXLINE, 0);
                string sixlog = info.getUUID() + ": " +  "Responding \"" + cacheres->responseLine + "\"\n";
                log.writeString(sixlog);
            }
        }
    }else{ //not in cache need add log

        // string test0 = "not found in cache";
        // log.writeString(test0);


        string twolog = info.getUUID() + ": " +  "not in cache\n";
        log.writeString(twolog);

        const char * host = request.host.c_str();
        const char * port = request.port.c_str();
        int serverfd = open_clientfd(host, port);
        if(serverfd < 0){
            printf("proxy to server Connection failed\n");
            //clienterror(fd, method, "400", "Bad Request",
                        //"Connection failed");
            return;
        }
        printf("open to server fd\nhost: %s, port: %s\n", host, port);


        vector<char> res(request.result.begin(), request.result.end());
        int len = send(serverfd, res.data(), MAXLINE, 0);

        string threelog = info.getUUID() + ": " +  "Requesting \"" + request.requestLine + "\" from " + request.host + "\n";
        log.writeString(threelog);

        //vector<char> buffer(MAXLINE);
        //vector<char> firstBuffer(MAXLINE);
        //vector<char> restBuffer;
        vector<char> firstbuffer;
        len = recv(serverfd, res.data(), res.size(), 0);
        

        string firstReponse = string(res.begin(), res.begin() + len);
        cout << "************************************************" << endl;
        //cout << "first response:\n" << firstReponse << endl;
        cout << "**********************************************************************************************" << endl;
        Response rp(firstReponse);

        string fourlog = info.getUUID() + ": " +  "Received \"" + rp.responseLine + "\" from " + request.host + "\n";
        log.writeString(fourlog);

        if(!rp.check_malformed()){
            //to do
            cout << "to do send 502 error code" << endl;
            send(clientfd, "HTTP/1.1 502 Bad Gateway\r\n\r\n", 28, 0);
            string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 502 Bad Gateway\"\n";
            log.writeString(sixlog);
            close(serverfd);
            return;
        }
        // cout << "********************************************************************************************" << endl;
        // cout << "first: " << rp.first << endl;
        // cout << "************************************************************************************" << endl;
        // cout << "responseLine: " << rp.responseLine << endl;
        // cout << "nocache: " << rp.nocache << endl;
        // cout << "ETag: " << rp.ETag << endl;
        // cout << "Lastmodified: " << rp.Lastmodified << endl;
        // cout << "maxage: " << rp.maxage << endl;
        // cout << "must revalidate: " << rp.mustrevalidate << endl;
        // cout << "expires: " << rp.expires << endl;
        // cout << "nostore: " << rp.nostore << endl;
        // cout << "private: " << rp.privat << endl;
        cout << "xuesen wen" << endl;
        send(clientfd, res.data(), len, 0);

        //added
        
        cout << "after xuese wen\n";
        //receive all
        vector<char> newbuffer;
        while((len = recv(serverfd, res.data(), res.size(), 0)) > 0){
            newbuffer.insert(newbuffer.end(), res.begin(), res.begin() + len);
            send(clientfd, res.data(), len, 0);
        }

        string restResponse = string(newbuffer.begin(), newbuffer.end());
        rp.rest = restResponse;


        cout << "xuesen 2\n";
        
        //cout << endl << restResponse << endl;
        cout << "end of response\n";
       
        string sixlog = info.getUUID() + ": " +  "Responding \"" + rp.responseLine +  "\"\n";
        log.writeString(sixlog);

        if(rp.checkToStore()){
            //to store
            // string test0 = "can store it\n";
            // log.writeString(test0);
            cache.put(request.requestLine, &rp, &log, info);
        }else{
            //not store
            string fivelog = info.getUUID() + ": " +  "not cacheable because nostore or private response header field\n";
            log.writeString(fivelog);
        }
        // string test0 = "not store it\n";
        // log.writeString(test0);
        cout << "iam here\n";
        close(serverfd);
    }
}

void handlePost(Request &request, int clientfd, Client_Info &info){
    const char * host = request.host.c_str();
        const char * port = request.port.c_str();
        int serverfd = open_clientfd(host, port);
        if(serverfd < 0){
            printf("proxy to server Connection failed\n");
            //clienterror(fd, method, "400", "Bad Request",
                        //"Connection failed");
            return;
        }
        printf("open to server fd\nhost: %s, port: %s\n", host, port);

        vector<char> res(request.result.begin(), request.result.end());
        int len = send(serverfd, res.data(), MAXLINE, 0);

        string threelog = info.getUUID() + ": " +  "Requesting \"" + request.requestLine + "\" from " + request.host + "\n";
        log.writeString(threelog);

        vector<char> firstbuffer;
        len = recv(serverfd, res.data(), res.size(), 0);
        send(clientfd, res.data(), len, 0);

        string firstReponse = string(res.begin(), res.begin() + len);
        Response rp(firstReponse);
        string fourlog = info.getUUID() + ": " +  "Received \"" + rp.responseLine + "\" from " + request.host + "\n";
        log.writeString(fourlog);
        if(!rp.check_malformed()){
            //to do
            cout << "to do send 502 error code" << endl;
            send(clientfd, "HTTP/1.1 502 Bad Gateway\r\n\r\n", 28, 0);
            string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 502 Bad Gateway\"\n";
            log.writeString(sixlog);
            close(serverfd);
            return;
        }

        vector<char> newbuffer;
        while((len = recv(serverfd, res.data(), res.size(), 0)) > 0){
            newbuffer.insert(newbuffer.end(), res.begin(), res.begin() + len);
            send(clientfd, res.data(), len, 0);
        }

        string restResponse = string(newbuffer.begin(), newbuffer.end());
        rp.rest = restResponse;

        string sixlog = info.getUUID() + ": " +  "Responding \"" + rp.responseLine +  "\"\n";
        log.writeString(sixlog);
        close(serverfd);
}

void handleConnect(Request &request, int clientfd, Client_Info &info){
    const char * host = request.host.c_str();
    int serverfd = open_clientfd(host, "443");
    if(serverfd < 0){
        printf("proxy to server Connection failed\n");
        //clienterror(fd, method, "400", "Bad Request",
                    //"Connection failed");
        return;
    }
    printf("open to server fd\nhost: %s, port: %s\n", host, "443");

    send(clientfd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 200 OK\"\n";
    log.writeString(sixlog);

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax = serverfd > clientfd? serverfd : clientfd;        // maximum file descriptor number

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(clientfd, &master);
    FD_SET(serverfd, &master);
    int len;
    int fd[2] = {serverfd, clientfd};
    while(true){
        read_fds = master;
        select(fdmax+1, &read_fds, NULL, NULL, NULL);
        for(int i = 0; i < 2; i++){
            char message[65536] = {0};
            if(FD_ISSET(fd[i], &read_fds)){
                len = recv(fd[i], message, sizeof(message), 0);
                if(len <= 0 ){
                    
                    return;
                }
                else{
                    if(send(fd[1-i], message, len, 0) <= 0){
                        
                        return;
                    }
                }
            }
        }
    }

}



/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(Client_Info info) 
{
    
    printf("%d\n", info.getFD());
    printf("before receive");
    printf("5\n");
    
    vector<char> buffer(MAXLINE);

    int len = recv(info.getFD(), buffer.data(), MAXLINE, 0);
    if (len <= 0){
        printf("read error\n");
        printf("6\n");
        return;
    }
    //log.writeString();
    string buf = string(buffer.begin(), buffer.end());
    //cout << buf << endl;
    Request request(buf);
    if(request.check_malformed_build()){
        string onelog = info.getUUID() + ": \"" + request.requestLine + "\" " + "from " + info.getIP() + " @ " + utctime();
        log.writeString(onelog);
    }else{
        //to do
        cout << "to do send 400 error code" << endl;
        send(info.getFD(), "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0);
        string sixlog = info.getUUID() + ": Responding \"HTTP/1.1 400 Bad Request\"\n";
        log.writeString(sixlog);
        return;
    }
    // printf("input:%s\n", request.input);
    // printf("7\n");
    // printf("uri:%s\n", request.uri);
    // printf("method:%s\n", request.method);
    // printf("host:%s\n", request.host);
    // printf("path:%s\n", request.path);
    // printf("port:%s\n", request.port);
    //cout << "input: " << request.input << endl;
    cout << "uri: " << request.uri << endl;
    cout << "method: " << request.method << endl;
    cout << "host: " << request.host << endl;
    cout << "path: " << request.path << endl;
    cout << "port: " << request.port << endl;

    // sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    // char url_store[100];
    // strcpy(url_store,uri);  /*store the original url */
    
    //info.setRequest(string(buf));
    if (request.method == "GET") {                     //line:netp:doit:beginrequesterr
        printf("handle get\nrequest to server:\n");
        //cout << request.result << endl;
        handleGet(request, info.getFD(), info);

    }else if(request.method == "POST"){
        printf("handle post\n request to server:\n");
        //cout << request.result << endl;
        handlePost(request, info.getFD(), info);
    }else if(request.method == "CONNECT"){
        printf("handle connect\n request to server:\n");
        //cout << request.result << endl;
        handleConnect(request, info.getFD(), info);
        string sevenlog = info.getUUID() + " Tunnel closed\n";
        log.writeString(sevenlog);
    }
    else{
        // todo send error
        // clienterror(fd, method, "501", "Not Implemented",
        //             "Tiny does not implement this method");
        // return;
        printf("handle error\n");
    }
    
    
}
/* $end doit */



void *thread(void *vargp){
    pthread_detach(pthread_self());
    while(1){
        cout << "enter while\n";
        Client_Info info = sbuf.subf_remove();
        printf("info fd:%d\n", info.getFD());
        //printf("6\n", info.getFD());
        doit(info);
	    close(info.getFD());
        cout << "after close\n";
    }
}

int main(int argc, char **argv){
    int listenfd = open_listenfd("12345");
    if(listenfd == -1){
        cout << "open listen fd failed\n";
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);
    pthread_t tid;
    for(int i = 0; i < NTHREADS;i++){
        pthread_create(&tid, NULL, thread, NULL);
    }

    int requestUUID = 0;
    while(1){
        string ip;
        int clientfd = accept_getip(listenfd, &ip);
        if(clientfd == -1){
            cout << "accept client fd failed\n";
            continue;
        }
        cout << "Accepted connection from (" << ip << ", " << ")\n";
        //printf("Accepted connection from (%s, %d)\n", ip, getport(clientfd));
	    Client_Info info(to_string(requestUUID), clientfd, ip);                                            //line:netp:tiny:close
        printf("1\n");
        printf("infofd:%d\n", info.getFD());
        sbuf.subf_insert(info);
        printf("2\n");
        requestUUID++;
        printf("UUID:%d", requestUUID);
        printf("3\n");
    }
}


/*
int main(int argc, char **argv){
    //string input = "Mon, 16 Dec 2019 21:07:41 GMT";
    string input = "Tue, 28 Feb 2023 17:34:00 GMT";
    struct tm res;
    parseHttpdate(input, &res);
    time_t pre = mktime(&res);

    //convert current time to UTC and print it
    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    char * dt = asctime(gmtm);
    cout << "The current UTC GMT date and time is:"<< dt << endl;
    now = mktime(gmtm);

    //calculate diff
    double dif = difftime(now, pre);
    cout << "dif:" << dif << endl;
    cout << "pre:" << pre << endl;

    time_t t1 = 1 + time(0);
    cout << utcconvert(t1);
    
    return 1;
}

*/
/*
int main(int argc, char **argv){
    cout << "start\n";
    Cache cache(2);
    cout << "here1\n";
    Response r1("11111");
    Response r2("22222");
    Response r3("33333");
    Response r4("44444");
    cout << "here2\n";
    Response* g1 = cache.get("1");  //should be nullptr
    if(g1 != nullptr){
        cout << "1:g1 wrong\n";
    }
    cout << "here3\n";
    cache.put("1", &r1);
    cout << "here4\n";
    g1 = cache.get("1"); //should be r1
    if(g1->first.compare("11111") != 0){
        cout << "2:g1 wrong\n";
    }
    cout << "here6\n";
    Response * g2 = cache.get("2"); // should be nullptr
    if(g2 != nullptr){
        cout << "3:g2 wrong\n";
    }
    cache.put("2", &r2);
    g2 = cache.get("2"); // should be r2
    if(g2->first.compare("22222") != 0){
        cout << "4:g2 wrong\n";
    }

    cache.put("3", &r3);
    Response * g3 = cache.get("3"); // should be r3
    if(g3->first.compare("33333") != 0){
        cout << "5:g3 wrong\n";
    }
    g1 = cache.get("1"); //should be nullptr
    if(g1 != nullptr){
        cout << "6:g1 wrong\n";
    }
    g2 = cache.get("2"); // should be r2
    if(g2->first.compare("22222") != 0){
        cout << "6:g2 wrong\n";
    }

    cout << "8:ok!!!!!!!!!!!!!!!!!!\n";
    //cout << g1->first << endl;
    cout << g2->first << endl;
    cout << g3->first << endl;

    return 1;
}
*/

/*
int main(int argc, char **argv){
    
    Log log;
    string s1("hihihihihihi!\n");
    log.writeString(s1);
    log.writeString(s1);
    log.closeFile();
    return 1;
}
*/
/*
if(dif <= finalage){
            return true; //use cache
        }else{
            //contact server
            const char * host = request.host.c_str();
            const char * port = request.port.c_str();
            int serverfd = open_clientfd(host, port);
            if(serverfd < 0){
                printf("proxy to server Connection failed\n");
                //clienterror(fd, method, "400", "Bad Request",
                            //"Connection failed");
                return false;
            }
            vector<char> newreqvec(request.input.begin(), request.input.end());
            int len = send(serverfd, newreqvec.data(), newreqvec.size(), 0);

            string threelog = info.getUUID() + ": " +  "Requesting " + request.requestLine + " from " + request.host;
            log.writeString(threelog);

            //receive from server
            vector<char> buffer(MAXLINE);
            vector<char> firstBuffer(MAXLINE);
            vector<char> restBuffer;
            len = recv(serverfd, firstBuffer.data(), MAXLINE, 0);
            if(len <= 0){
                cout << "failed first receive from server on data\n";
                return false;
            }

            //new add
            string validateres = string(firstBuffer.begin(), firstBuffer.end());
    
            Response rp(validateres);
            if(!rp.check_malformed()){
                //to do
                cout << "to do send 502 error code" << endl;
                return false;
            }
            //send first part
            send(clientfd, firstBuffer.data(), MAXLINE, 0);
            //receive all from server and send all to client
            while((len = recv(serverfd, buffer.data(), MAXLINE, 0)) != 0){
                restBuffer.insert(restBuffer.end(), buffer.begin(), buffer.begin() + len);
                send(clientfd, buffer.data(), MAXLINE, 0);
            }
            string restResponse = string(restBuffer.begin(), restBuffer.end());
            cout << endl << restResponse << endl;
            cout << "end of response\n";
            rp.rest = restResponse;
            
            string fourlog = info.getUUID() + ": " +  "Received " + rp.responseLine + " from " + request.host;
            log.writeString(fourlog);
            string sixlog = info.getUUID() + ": " +  "Responding" + rp.responseLine;
            log.writeString(sixlog);
            //check and  to store in cache
            if(rp.checkToStore()){
                cache.put(request.requestLine, &rp, &log, info);
            }else{
                string fivelog = info.getUUID() + ": " +  "not cacheable because response contain \"nostore\" or \"private\"";
                log.writeString(fivelog);
            }
        }
        */