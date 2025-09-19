#include <stdio.h>

#include "utils.h"
#include "log.h"

int check_success(int test_case, const char* error_msg) {
    if (test_case < 0) {
        print_log("%s", error_msg);
        return FAIL;
    }

    return SUCCESS;
}
