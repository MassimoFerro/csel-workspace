#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define LED_GPIO "10"
#define K1_GPIO "0"
#define K2_GPIO "2"
#define K3_GPIO "3"

#define GPIO_PATH "/sys/class/gpio"
#define MAX_EVENTS 4

int gpio_export(const char *gpio) {
    int fd = open(GPIO_PATH "/export", O_WRONLY);
    if (fd < 0) return -1;
    write(fd, gpio, strlen(gpio));
    close(fd);
    return 0;
}

int gpio_set_dir(const char *gpio, const char *dir) {
    char path[128];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%s/direction", gpio);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

int gpio_set_edge(const char *gpio, const char *edge) {
    char path[128];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%s/edge", gpio);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    write(fd, edge, strlen(edge));
    close(fd);
    return 0;
}

int gpio_open_value_fd(const char *gpio, int flags) {
    char path[128];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%s/value", gpio);
    return open(path, flags);
}

void update_timer(int tfd, int freq) {
    struct itimerspec timer;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 500000000 / freq;
    timer.it_value = timer.it_interval;
    timerfd_settime(tfd, 0, &timer, NULL);
}

int main() {
    openlog("led_control", LOG_PID | LOG_CONS, LOG_USER);

    // GPIO Setup
    gpio_export(LED_GPIO);
    gpio_export(K1_GPIO);
    gpio_export(K2_GPIO);
    gpio_export(K3_GPIO);

    gpio_set_dir(LED_GPIO, "out");
    gpio_set_dir(K1_GPIO, "in");
    gpio_set_dir(K2_GPIO, "in");
    gpio_set_dir(K3_GPIO, "in");

    gpio_set_edge(K1_GPIO, "rising");
    gpio_set_edge(K2_GPIO, "rising");
    gpio_set_edge(K3_GPIO, "rising");

    int led_fd = gpio_open_value_fd(LED_GPIO, O_WRONLY);
    int k1_fd = gpio_open_value_fd(K1_GPIO, O_RDONLY | O_NONBLOCK);
    int k2_fd = gpio_open_value_fd(K2_GPIO, O_RDONLY | O_NONBLOCK);
    int k3_fd = gpio_open_value_fd(K3_GPIO, O_RDONLY | O_NONBLOCK);

    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

    int epfd = epoll_create1(0);

    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = timer_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, timer_fd, &ev);

    ev.events = EPOLLPRI | EPOLLET;
    ev.data.fd = k1_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, k1_fd, &ev);

    ev.data.fd = k2_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, k2_fd, &ev);

    ev.data.fd = k3_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, k3_fd, &ev);

    int freq = 2;
    update_timer(timer_fd, freq);
    syslog(LOG_INFO, "Start with %d Hz", freq);

    int led_state = 0;

    struct epoll_event events[MAX_EVENTS];
    char buf[8];

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == timer_fd) {
                uint64_t expirations;
                read(timer_fd, &expirations, sizeof(expirations));
                led_state = !led_state;
                dprintf(led_fd, "%d", led_state);
            } else {
                if (events[i].data.fd == k1_fd) {
                    lseek(k1_fd, 0, SEEK_SET);
                    read(k1_fd, buf, sizeof(buf));
                    freq++;
                    syslog(LOG_INFO, "Frequency increased to %d Hz", freq);
                } else if (events[i].data.fd == k2_fd) {
                    lseek(k2_fd, 0, SEEK_SET);
                    read(k2_fd, buf, sizeof(buf));
                    freq = 2;
                    syslog(LOG_INFO, "Frequency reset to %d Hz", freq);
                } else if (events[i].data.fd == k3_fd) {
                    lseek(k3_fd, 0, SEEK_SET);
                    read(k3_fd, buf, sizeof(buf));
                    if (freq > 1) freq--;
                    syslog(LOG_INFO, "Frequency decreased to %d Hz", freq);
                }
                update_timer(timer_fd, freq);
            }
        }
    }

    close(led_fd);
    close(k1_fd);
    close(k2_fd);
    close(k3_fd);
    close(timer_fd);
    close(epfd);
    closelog();
    return 0;
}
