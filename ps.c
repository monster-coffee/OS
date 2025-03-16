#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

void list_processes() {
    struct dirent *entry;
    DIR *proc_dir = opendir("/proc");

    if (!proc_dir) {
        perror("Failed to open /proc");
        exit(EXIT_FAILURE);
    }

    printf("%-10s %-20s\n", "PID", "COMMAND");
    printf("----------------------------\n");

    while ((entry = readdir(proc_dir)) != NULL) {
        if (!isdigit(entry->d_name[0]))  // Skip non-numeric entries
            continue;

        char cmd_path[256], cmd[256];
        FILE *cmd_file;

        snprintf(cmd_path, sizeof(cmd_path), "/proc/%s/cmdline", entry->d_name);
        cmd_file = fopen(cmd_path, "r");

        if (cmd_file) {
            if (fgets(cmd, sizeof(cmd), cmd_file) != NULL) {
                printf("%-10s %-20s\n", entry->d_name, cmd);
            } else {
                printf("%-10s [Unknown]\n", entry->d_name);
            }
            fclose(cmd_file);
        } else {
            printf("%-10s [Unknown]\n", entry->d_name);
        }
    }

    closedir(proc_dir);
}

int main() {
    list_processes();
    return EXIT_SUCCESS;
}
