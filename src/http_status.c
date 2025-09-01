#include "http_status.h"
#include <stdio.h>

const char *get_reason_phrase(int code) {
    for (int i = 0; i < http_statuses_count; i++) {
        if (http_statuses[i].code == code) {
            return http_statuses[i].reason;
        }
    }
    return "Unknown Status";
}
