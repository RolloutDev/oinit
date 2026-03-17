// oinit.c
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mount.h>

#define SERVICE_DIR "/etc/oinit.d"
#define SHELL "/bin/sh"

int main(void) {
    printf("oinit 0.pre-alpha starting...\n");

    // Optional mounts
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);
    mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);

    DIR *dir = opendir(SERVICE_DIR);
    if (!dir) {
        perror("opendir");
        goto shell;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[256];
        snprintf(path, sizeof(path), "%s/%s", SERVICE_DIR, entry->d_name);

        printf("Starting service: %s\n", entry->d_name);

        pid_t pid = fork();
        if (pid == 0) {
            // child process
            execl(path, path, (char*)NULL);
            perror("execl failed");
            _exit(1);
        } else if (pid > 0) {
            // parent waits
            int status;
            waitpid(pid, &status, 0);
            printf("Finished service: %s\n", entry->d_name);
        } else {
            perror("fork failed");
        }
    }

    closedir(dir);

shell:
    printf("All services executed. Dropping to shell...\n");
    execl(SHELL, SHELL, (char*)NULL);
    perror("Failed to start shell");
    return 1;
}
