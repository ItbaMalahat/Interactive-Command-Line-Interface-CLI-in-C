
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE_LENGTH 1024

// Function declarations
int my_system(const char *command);
void sigchld_handler(int signo);

int main() {
    // Set up the SIGCHLD handler to reap zombie processes
    signal(SIGCHLD, sigchld_handler);

    char line[MAX_LINE_LENGTH];

    while (1) {
        // Prompt for user input
        printf("> ");

        // Read user input
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL) {
            // Handle errors or EOF
            break;
        }

        // Remove trailing newline (if present)
        line[strcspn(line, "\n")] = '\0';

        // Check for the exit command
        if (strcmp(line, "exit") == 0) {
            break;
        }

        // Execute the command using my_system
        int ret = my_system(line);
        if (ret == -1) {
            printf("Error executing command\n");
        } else {
            printf("Command exited with status: %d\n", ret);
        }
    }

    // Print an exit message
    printf("Exiting...\n");
    return 0;
}

// Custom system function
int my_system(const char *command) {
    pid_t pid;
    int status;

    // Check if the command ends with '&'
    int background = 0;
    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '&') {
        background = 1;
        // Remove '&' from the command
        char *cmd_copy = strdup(command);
        cmd_copy[len - 1] = '\0';
        command = cmd_copy;
    }

    // Split the command and arguments into an array (maximum 128 arguments)
    char *argv[128];
    int argc = 0;
    char *token = strtok((char *)command, " "); // Tokenize the command string
    while (token != NULL) {
        argv[argc++] = token; // Add each token (argument) to the argv array
        token = strtok(NULL, " "); // Get the next token
    }
    argv[argc] = NULL; // Terminate the argument list with NULL

    // Fork a child process
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        // Child process: execute the command using execvp
        execvp(argv[0], argv);
        // If execvp fails, print an error and exit
        perror("execvp failed");
        exit(1);
    } else {
        // Parent process: wait for child process to finish (if not running in the background)
        if (!background) {
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status); // Return child's exit status
            } else {
                return -1; // Child process terminated abnormally
            }
        }
    }

    return 0;
}

// SIGCHLD handler to reap zombie processes
void sigchld_handler(int signo) {
    (void)signo;

    // Reap all zombie child processes
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}
