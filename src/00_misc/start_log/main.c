#include <stdio.h>
#include <time.h>

#define LOG_FILE "startup.log"

void log_message() {
    FILE *file = fopen(LOG_FILE, "a");
    if (file == NULL) {
        perror("Error opening log file");
        return;
    }

    // Get the current date and time
    time_t now;
    struct tm *time_info;
    char time_buffer[50];

    time(&now);
    time_info = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

    // Write to log file
    fprintf(file, "[%s] Startup script executed successfully.\n", time_buffer);
    fclose(file);
}

int main() {
    log_message();
    return 0;
}
