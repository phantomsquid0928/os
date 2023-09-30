#include "types.h"
#include "stat.h"
#include "user.h"
#include "date.h"

int main(int argc, char *argv[]) {
    struct rtcdate r;
    if (argc != 1) {
        printf(1, "usage : datetest\n");
        exit();
    }
    if (date(&r) < 0) {
        printf(1, "datetest : fetching rtcdate occured error\n");
        exit();
    }
    printf(1, "Current time : %d-%d-%d %d:%d:%d\n", r.year, r.month, r.day, r.hour, r.minute, r.second);
    
    exit();
}