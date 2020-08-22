// NAME: Aayush Patel
// EMAIL: aayushpatel108@ucla.edu
// UID: 105111196

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define READ 0
#define WRITE 1
#define BUFFER_SIZE 256

struct termios saved_attributes;
pid_t pid;
int child_pipe[2];
int parent_pipe[2];
static int shell_flag = 0;

void reset_input_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode() {
    struct termios t_attr;

    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not a terminal. \n");
        exit(EXIT_FAILURE);
    }

    tcgetattr(STDIN_FILENO, &saved_attributes);
    atexit(reset_input_mode);

    tcgetattr(STDIN_FILENO, &t_attr);
    t_attr.c_iflag = ISTRIP;
    t_attr.c_oflag = 0;
    t_attr.c_lflag = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_attr);
}

void shutdown_processing() {
    int sig;
    if ((waitpid(pid, &sig, 0)) == -1) {
        fprintf(stderr, "Error during waitpid. Could not get error status\n");
        exit(1);
    }
    int signal = WTERMSIG(sig);
    int status = WEXITSTATUS(sig);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", signal, status);
    exit(0);
}

void child() {
    close(child_pipe[READ]);
    close(parent_pipe[WRITE]);

    close(STDIN_FILENO);
    dup(parent_pipe[READ]);
    close(parent_pipe[READ]);

    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    dup(child_pipe[WRITE]);
    dup(child_pipe[WRITE]);
    close(child_pipe[WRITE]);

    char *arg[] = {"/bin/bash", NULL};
    if (execvp(arg[0], arg) == -1) {
        fprintf(stderr, "Execvp failed");
        exit(1);
    }
    fprintf(stdout, "started shell");
}

int read_from_keyboard() {
    char buf[BUFFER_SIZE];
    int bytes_remaining;
    int should_exit = 0;
    if ((bytes_remaining = read(STDIN_FILENO, buf, sizeof(char) * BUFFER_SIZE)) >= 0) {
        int i;
        for (i = 0; i < bytes_remaining; i++) {
            switch (buf[i]) {
                case 0x03:
                    write(STDOUT_FILENO, "^C", sizeof(char) * 2);
                    if (shell_flag)
                        kill(pid, SIGINT);
                    break;
                case 0x04:
                    write(STDOUT_FILENO, "^D", sizeof(char) * 2);
                    if (shell_flag) {
                        close(parent_pipe[WRITE]);
                        should_exit = 1;
                    } else {
                        exit(0);
                    }
                    break;
                case '\r':
                case '\n':
                    write(STDOUT_FILENO, "\r\n", sizeof(char) * 2);
                    if (shell_flag)
                        write(parent_pipe[WRITE], "\n", sizeof(char));
                    break;
                default:
                    write(STDOUT_FILENO, &buf[i], sizeof(char));
                    if (shell_flag)
                        write(parent_pipe[WRITE], &buf[i], sizeof(char));
            }
        }
    } else {
        fprintf(stderr, "Could not read from keyboard\n");
        exit(1);
    }
    return should_exit;
}

int read_from_shell() {
    char buf[BUFFER_SIZE];
    int bytes_remaining;
    int should_exit = 0;
    if ((bytes_remaining = read(child_pipe[READ], buf, sizeof(char) * BUFFER_SIZE)) >= 0) {
        int i;
        for (i = 0; i < bytes_remaining; i++){
            switch (buf[i]) {
                case 0x04:
                    should_exit = 1;
                    break;
                case '\n':
                    write(STDOUT_FILENO, "\r\n", sizeof(char) * 2);
                    break;
                default:
                    write(STDOUT_FILENO, &buf[i], sizeof(char));
            }
        }
    } else {
        fprintf(stderr, "Could not read from shell\n");
        exit(1);
    }
    return should_exit;
}

void parent() {
    close(child_pipe[WRITE]);
    close(parent_pipe[READ]);

    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN | POLLERR | POLLHUP;

    fds[1].fd = child_pipe[READ];
    fds[1].events = POLLIN | POLLERR | POLLHUP;

    int should_exit = 0;
    while (!should_exit) {

        if (poll(fds, 2, 0) == -1) {
            fprintf(stderr, "Error during poll\n");
            exit(1);
        }

        if (fds[0].revents & POLLIN)
            should_exit = read_from_keyboard();

        if (fds[1].revents & POLLIN) 
            should_exit |= read_from_shell();

        short error = POLLERR | POLLHUP;

        if (fds[1].revents & error) 
            break;
    }
    shutdown_processing();
}

int main(int argc, char **argv) {
    int c;
    static struct option long_options[] = {
        {"shell", no_argument, &shell_flag, 1},
        {0, 0, 0, 0}};

    if ((c = getopt_long(argc, argv, "", long_options, NULL)) == '?') {
        fprintf(stderr, "Unrecognized argument. Only one optional argument: --shell\n");
        exit(1);
    }

    set_input_mode();

    if (shell_flag) {

        if (pipe(child_pipe) == -1) {
            fprintf(stderr, "Error creating pipe.\n");
            exit(1);
        }
        if (pipe(parent_pipe) == -1) {
            fprintf(stderr, "Error creating pipe.\n");
            exit(1);
        }

        pid = fork();
        if (pid == 0) {
            child();
        } else if (pid > 0) {
            parent();
        } else {
            fprintf(stderr, "Error during fork.\n");
            exit(1);
        }
    } else {
        while (1) {
            if (read_from_keyboard())
                break;
        }
    }
    exit(0);
}
