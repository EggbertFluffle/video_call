#include <stdio.h>

#define SUCCESS 0
#define FAIL -1

#define TRUE 1
#define FALSE 0

#define EMPTY ""

int check_success(int test_case, const char* error_msg) {
    if (test_case < 0) {
        printf("%s", error_msg);
        return FAIL;
    }

    return SUCCESS;
}
