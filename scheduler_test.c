#include "types.h"
#include "stat.h"
#include "user.h"

void scheduler_func(void) {
    set_sched_info(getpid(), 11);
    int pinputs[3] = {1, 11, 13};
    for (int i = 0;i < 3; i++) {
        int pid = fork();
        if (pid < 0) break;
        if (pid == 0) {
            alarm(10);
            set_sched_info(getpid(), pinputs[i]);
            while(1) {}
        }
    }
    for (int i = 0; i < 3; i++) {
        wait();
    }
    // for (int i = 0; i< 5; i++) {
    //     int pid = fork();
    //     printf(1, "%d", pid);
    //     sleep(3);
    //     if (pid > 0) {
    //         wait();
    //     }
    // }
    // for (int i = 0;i < 5; i++) {
    //     int pid = fork();
    //     printf(1, "%d", pid);
    //     if (pid < 0) exit();
    //     if (pid == 0) { //자식?
    //         alarm(2 * (i + 1));
    //         while(1) {

    //         }
    //     }
    //     else {         //부모
    //         int wpid = wait();
    //         printf(1, "t");
    //         printf(1, "wpid : %d %d", wpid);
    //     }
    // }
}

int main(void) {
    scheduler_func();
    exit();
}