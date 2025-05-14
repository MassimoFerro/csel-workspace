#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void burn_cpu() {
    while (1) {
        // Infinite loop to consume CPU
    }
}

int main() {
    pid_t p1 = fork();

    if (p1 == 0) {
        burn_cpu(); // First child
    }

    pid_t p2 = fork();

    if (p2 == 0) {
        burn_cpu(); // Second child
    }

    // Parent waits forever (optional)
    wait(NULL);
    wait(NULL);

    return 0;
}
