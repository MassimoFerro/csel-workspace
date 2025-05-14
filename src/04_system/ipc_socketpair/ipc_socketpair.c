#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define MAX_MSG_LEN 256

void handle_signal(int sig) {
    printf("Signal %d reçu et ignoré\n", sig);
}

void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    int signals[] = {SIGHUP, SIGINT, SIGQUIT, SIGABRT, SIGTERM};
    for (int i = 0; i < 5; ++i) {
        sigaction(signals[i], &sa, NULL);
    }
}

void set_affinity(int core_id) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_id, &set);
    if (sched_setaffinity(0, sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
    }
}

int main() {
    int sv[2]; // socket pair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    setup_signals(); // install signal handlers

    if (pid == 0) {
        // CHILD PROCESS
        close(sv[0]); // close parent's end
        set_affinity(1); // assign to core 1

        const char* messages[] = {
            "Message 1 : Bonjour parent !",
            "Message 2 : Je fonctionne sur core 1",
            "Message 3 : Tout va bien",
            "exit"
        };

        for (int i = 0; i < 4; ++i) {
            sleep(1);
            write(sv[1], messages[i], strlen(messages[i]) + 1);
        }

        close(sv[1]);
        exit(0);
    } else {
        // PARENT PROCESS
        close(sv[1]); // close child's end
        set_affinity(0); // assign to core 0

        char buffer[MAX_MSG_LEN];
        while (1) {
            ssize_t n = read(sv[0], buffer, MAX_MSG_LEN);
            if (n > 0) {
                if (strcmp(buffer, "exit") == 0) {
                    printf("Reçu 'exit', arrêt du parent.\n");
                    break;
                } else {
                    printf("Parent a reçu : %s\n", buffer);
                }
            }
        }

        close(sv[0]);
        wait(NULL);
    }

    return 0;
}
