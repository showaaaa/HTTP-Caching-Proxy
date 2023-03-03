http://csapp.cs.cmu.edu/3e/labs.html
curl -v --proxy http://localhost:12345 http://csapp.cs.cmu.edu/3e/labs.html

http://www.httpwatch.com/httpgallery/chunked/chunkedimage.aspx
http://www.columbia.edu/~fdc/sample.html


in-cache valid (no expire & no max-age): http://www.columbia.edu/~fdc/sample.html
no-store: http://httpbin.org/response-headers?freeform=Cache-Control%3A%20no-store
no-cache: http://httpbin.org/response-headers?freeform=Cache-Control%3A%20no-cache
private: http://httpbin.org/response-headers?freeform=Cache-Control%3A%20private
max-age: http://httpbin.org/cache/20 （expires in 20s）



expires: curl -v --proxy http://localhost:12346 http://httpbin.org/response-headers?freeform=Expires%3A%20Fri%2C%2003%20Mar%202023%2021%3A40%3A00%20GMT