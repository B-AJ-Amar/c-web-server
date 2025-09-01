#ifndef HTTP_STATUS_H
#define HTTP_STATUS_H

#define HTTP_CONTINUE                        100
#define HTTP_SWITCHING_PROTOCOLS             101
#define HTTP_PROCESSING                      102

#define HTTP_OK                              200
#define HTTP_CREATED                         201
#define HTTP_ACCEPTED                        202
#define HTTP_NON_AUTHORITATIVE_INFORMATION   203
#define HTTP_NO_CONTENT                      204
#define HTTP_RESET_CONTENT                   205
#define HTTP_PARTIAL_CONTENT                 206
#define HTTP_MULTI_STATUS                    207
#define HTTP_ALREADY_REPORTED                208
#define HTTP_IM_USED                         226

#define HTTP_MULTIPLE_CHOICES                300
#define HTTP_MOVED_PERMANENTLY               301
#define HTTP_FOUND                           302
#define HTTP_SEE_OTHER                       303
#define HTTP_NOT_MODIFIED                    304
#define HTTP_USE_PROXY                       305
#define HTTP_TEMPORARY_REDIRECT              307
#define HTTP_PERMANENT_REDIRECT              308

#define HTTP_BAD_REQUEST                     400
#define HTTP_UNAUTHORIZED                    401
#define HTTP_PAYMENT_REQUIRED                402
#define HTTP_FORBIDDEN                       403
#define HTTP_NOT_FOUND                       404
#define HTTP_METHOD_NOT_ALLOWED              405
#define HTTP_NOT_ACCEPTABLE                  406
#define HTTP_PROXY_AUTHENTICATION_REQUIRED   407
#define HTTP_REQUEST_TIMEOUT                 408
#define HTTP_CONFLICT                        409
#define HTTP_GONE                            410
#define HTTP_LENGTH_REQUIRED                 411
#define HTTP_PRECONDITION_FAILED             412
#define HTTP_PAYLOAD_TOO_LARGE               413
#define HTTP_URI_TOO_LONG                    414
#define HTTP_UNSUPPORTED_MEDIA_TYPE          415
#define HTTP_RANGE_NOT_SATISFIABLE           416
#define HTTP_EXPECTATION_FAILED              417
#define HTTP_IM_A_TEAPOT                     418
#define HTTP_MISDIRECTED_REQUEST             421
#define HTTP_UNPROCESSABLE_ENTITY            422
#define HTTP_LOCKED                          423
#define HTTP_FAILED_DEPENDENCY               424
#define HTTP_TOO_EARLY                       425
#define HTTP_UPGRADE_REQUIRED                426
#define HTTP_PRECONDITION_REQUIRED           428
#define HTTP_TOO_MANY_REQUESTS               429
#define HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define HTTP_UNAVAILABLE_FOR_LEGAL_REASONS   451

#define HTTP_INTERNAL_SERVER_ERROR           500
#define HTTP_NOT_IMPLEMENTED                 501
#define HTTP_BAD_GATEWAY                     502
#define HTTP_SERVICE_UNAVAILABLE             503
#define HTTP_GATEWAY_TIMEOUT                 504
#define HTTP_HTTP_VERSION_NOT_SUPPORTED      505
#define HTTP_VARIANT_ALSO_NEGOTIATES         506
#define HTTP_INSUFFICIENT_STORAGE            507
#define HTTP_LOOP_DETECTED                   508
#define HTTP_NOT_EXTENDED                    510
#define HTTP_NETWORK_AUTHENTICATION_REQUIRED 511

struct http_status {
    int code;
    const char *reason;
};

static const struct http_status http_statuses[] = {
    {HTTP_CONTINUE,                        "Continue"},
    {HTTP_SWITCHING_PROTOCOLS,             "Switching Protocols"},
    {HTTP_PROCESSING,                      "Processing"},
    
    {HTTP_OK,                              "OK"},
    {HTTP_CREATED,                         "Created"},
    {HTTP_ACCEPTED,                        "Accepted"},
    {HTTP_NON_AUTHORITATIVE_INFORMATION,   "Non-Authoritative Information"},
    {HTTP_NO_CONTENT,                      "No Content"},
    {HTTP_RESET_CONTENT,                   "Reset Content"},
    {HTTP_PARTIAL_CONTENT,                 "Partial Content"},
    {HTTP_MULTI_STATUS,                    "Multi-Status"},
    {HTTP_ALREADY_REPORTED,                "Already Reported"},
    {HTTP_IM_USED,                         "IM Used"},
    
    {HTTP_MULTIPLE_CHOICES,                "Multiple Choices"},
    {HTTP_MOVED_PERMANENTLY,               "Moved Permanently"},
    {HTTP_FOUND,                           "Found"},
    {HTTP_SEE_OTHER,                       "See Other"},
    {HTTP_NOT_MODIFIED,                    "Not Modified"},
    {HTTP_USE_PROXY,                       "Use Proxy"},
    {HTTP_TEMPORARY_REDIRECT,              "Temporary Redirect"},
    {HTTP_PERMANENT_REDIRECT,              "Permanent Redirect"},
    
    {HTTP_BAD_REQUEST,                     "Bad Request"},
    {HTTP_UNAUTHORIZED,                    "Unauthorized"},
    {HTTP_PAYMENT_REQUIRED,                "Payment Required"},
    {HTTP_FORBIDDEN,                       "Forbidden"},
    {HTTP_NOT_FOUND,                       "Not Found"},
    {HTTP_METHOD_NOT_ALLOWED,              "Method Not Allowed"},
    {HTTP_NOT_ACCEPTABLE,                  "Not Acceptable"},
    {HTTP_PROXY_AUTHENTICATION_REQUIRED,   "Proxy Authentication Required"},
    {HTTP_REQUEST_TIMEOUT,                 "Request Timeout"},
    {HTTP_CONFLICT,                        "Conflict"},
    {HTTP_GONE,                            "Gone"},
    {HTTP_LENGTH_REQUIRED,                 "Length Required"},
    {HTTP_PRECONDITION_FAILED,             "Precondition Failed"},
    {HTTP_PAYLOAD_TOO_LARGE,               "Payload Too Large"},
    {HTTP_URI_TOO_LONG,                    "URI Too Long"},
    {HTTP_UNSUPPORTED_MEDIA_TYPE,          "Unsupported Media Type"},
    {HTTP_RANGE_NOT_SATISFIABLE,           "Range Not Satisfiable"},
    {HTTP_EXPECTATION_FAILED,              "Expectation Failed"},
    {HTTP_IM_A_TEAPOT,                     "I'm a teapot"},
    {HTTP_MISDIRECTED_REQUEST,             "Misdirected Request"},
    {HTTP_UNPROCESSABLE_ENTITY,            "Unprocessable Entity"},
    {HTTP_LOCKED,                          "Locked"},
    {HTTP_FAILED_DEPENDENCY,               "Failed Dependency"},
    {HTTP_TOO_EARLY,                       "Too Early"},
    {HTTP_UPGRADE_REQUIRED,                "Upgrade Required"},
    {HTTP_PRECONDITION_REQUIRED,           "Precondition Required"},
    {HTTP_TOO_MANY_REQUESTS,               "Too Many Requests"},
    {HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
    {HTTP_UNAVAILABLE_FOR_LEGAL_REASONS,   "Unavailable For Legal Reasons"},
    
    {HTTP_INTERNAL_SERVER_ERROR,           "Internal Server Error"},
    {HTTP_NOT_IMPLEMENTED,                 "Not Implemented"},
    {HTTP_BAD_GATEWAY,                     "Bad Gateway"},
    {HTTP_SERVICE_UNAVAILABLE,             "Service Unavailable"},
    {HTTP_GATEWAY_TIMEOUT,                 "Gateway Timeout"},
    {HTTP_HTTP_VERSION_NOT_SUPPORTED,      "HTTP Version Not Supported"},
    {HTTP_VARIANT_ALSO_NEGOTIATES,         "Variant Also Negotiates"},
    {HTTP_INSUFFICIENT_STORAGE,            "Insufficient Storage"},
    {HTTP_LOOP_DETECTED,                   "Loop Detected"},
    {HTTP_NOT_EXTENDED,                    "Not Extended"},
    {HTTP_NETWORK_AUTHENTICATION_REQUIRED, "Network Authentication Required"}
};

static const int http_statuses_count = sizeof(http_statuses) / sizeof(http_statuses[0]);

const char* get_reason_phrase(int code);

#endif // HTTP_STATUS_H
