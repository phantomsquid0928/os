#include "types.h"
#include "stat.h"
#include "user.h"

void scheduler_func(void) {
    pid = fork();
}

int main(void) {
    scheduler_func();
    exit();
}