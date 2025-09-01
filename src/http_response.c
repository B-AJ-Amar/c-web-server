/*

<HTTP-Version> <Status-Code> <Reason-Phrase>\r\n
<Header-Name>: <Header-Value>\r\n
<Header-Name>: <Header-Value>\r\n
...\r\n
\r\n
<Body>



headers:
server , date , conent-len , content-type  keep-alive(false)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "http_parser.h"
#include "http_status.h"
#include "http_response.h"


char* get_http_date() {
    time_t now = time(NULL);
    struct tm gm_time;
    gmtime_r(&now, &gm_time);

    char *buffer = malloc(100);
    if (!buffer) return NULL;

    strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", &gm_time);
    return buffer; 
}

void set_content_type(http_response *res, const char *ext) {

    if (strcmp(ext, "html") == 0) res->content_type =  CONTENT_TYPES.html;
    else if (strcmp(ext, "css") == 0) res->content_type =  CONTENT_TYPES.css;
    else if (strcmp(ext, "js") == 0) res->content_type =  CONTENT_TYPES.js;
    else if (strcmp(ext, "json") == 0) res->content_type =  CONTENT_TYPES.json;
    else if (strcmp(ext, "png") == 0) res->content_type =  CONTENT_TYPES.png;
    else if (strcmp(ext, "jpg") == 0) res->content_type =  CONTENT_TYPES.jpg;
    else if (strcmp(ext, "gif") == 0) res->content_type =  CONTENT_TYPES.gif;
    else if (strcmp(ext, "svg") == 0) res->content_type =  CONTENT_TYPES.svg;
    else res->content_type =  CONTENT_TYPES.txt;
}

void generateResponse(http_request* req,http_response* res,int status){
    res->status = status;
    res->reason = (char*)get_reason_phrase(status);
    res->version = req->version;
    res->date = get_http_date();

}


void generateFileResponse(http_request* req, http_response *res){

    // get the file based on the endpoint

    // res->body = content;
    // res->content_len = strlen(content);


    // generateResponse(req, res, status);


}



char* parseResponse(http_response* res){
    char* response = (char*)malloc(res->content_len + 512); // 512 for headers
    sprintf(response, "%s %d %s\r\n", res->version , res->status, res->reason);
    // Add the main headers
    sprintf(response + strlen(response), "Server: %s\r\n", SERVER_NAME);
    sprintf(response + strlen(response), "Content-Length: %d\r\n", res->content_len);
    sprintf(response + strlen(response), "Content-Type: %s\r\n", res->content_type);
    sprintf(response + strlen(response), "Date: %s\r\n", res->date);

    // TODO: add extra headers_suport
    
    sprintf(response + strlen(response), "\r\n%s", res->body);
    return response;

}