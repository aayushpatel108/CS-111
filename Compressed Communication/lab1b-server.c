// NAME: Aayush Patel
// EMAIL: aayushpatel108@ucla.edu
// UID: 105111196

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "zlib.h"

#define CHUNK 16384

#define READ 0
#define WRITE 1
#define BUFFER_SIZE 256

pid_t pid;
int child_pipe[2];
int parent_pipe[2];
int sockfd, newsockfd, portno;
socklen_t clilen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
int n;
int compress_flag = 0;

void shutdown_processing() {
    int sig;
    if ((waitpid(pid, &sig, 0)) == -1) {
        fprintf(stderr, "Error during waitpid. Could not get error status\r\n");
        exit(1);
    }
    int signal = WTERMSIG(sig);
    int status = WEXITSTATUS(sig);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", signal, status);
    close(newsockfd);
    close(sockfd);
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
        fprintf(stderr, "Execvp failed\r\n");
        exit(1);
    }
}

int inflate_to_socket(char* buf, int bytes_in) {
    int should_exit = 0;
    int ret;
    z_stream strm;
    unsigned char out[CHUNK];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.avail_in = bytes_in;
    strm.next_in = (unsigned char *) buf;
    strm.avail_out = CHUNK;
    strm.next_out = out;

    if ((ret = inflateInit(&strm)) != Z_OK)
        return ret;
    do {
        inflate(&strm, Z_SYNC_FLUSH);
    } while (strm.avail_in != 0);
    
    int bytes_after_inflation = (int) CHUNK - strm.avail_out;
    int i;
    for (i = 0; i < bytes_after_inflation; i++) {
       switch (out[i]) {
           case 0x03:
               write(newsockfd, "^C", sizeof(char) * 2);
               kill(pid, SIGINT);
               break;
           case 0x04:
               write(newsockfd, "^D", sizeof(char) * 2);
               close(parent_pipe[WRITE]);
               should_exit = 1;
               break;
           case '\r':
           case '\n':
               write(parent_pipe[WRITE], "\n", sizeof(char));
               break;
           default:
               write(parent_pipe[WRITE], &out[i], sizeof(char));
       }
    }
    (void)inflateEnd(&strm);
    return should_exit;
}

int read_from_socket() {
    char buf[BUFFER_SIZE];
    int bytes_read;
    int should_exit = 0;
    if ((bytes_read = read(newsockfd, buf, sizeof(char) * BUFFER_SIZE)) == 0) {
        kill(pid, SIGTERM);
    } else if (bytes_read < 0){
        fprintf(stderr, "Could not read from socket");
        kill(pid, SIGTERM);
    } else {
        if (compress_flag) {
            should_exit |= inflate_to_socket(buf, bytes_read);
        } else {
            int i;
            for (i = 0; i < bytes_read; i++) {
               switch (buf[i]) {
                   case 0x03:
                       kill(pid, SIGINT);
                       break;
                   case 0x04:
                       close(parent_pipe[WRITE]);
                       should_exit = 1;
                       break;
                   case '\r':
                   case '\n':
                       write(parent_pipe[WRITE], "\n", sizeof(char));
                       break;
                   default:
                       write(parent_pipe[WRITE], &buf[i], sizeof(char));
               }
            }
        }
    }
    return should_exit;
}

int deflate_to_socket(char* buf, int bytes_in) {
    int ret;
    z_stream strm;
    unsigned char out[CHUNK];
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    strm.avail_in = bytes_in;
    strm.next_in = (unsigned char *) buf;
    strm.avail_out = CHUNK;
    strm.next_out = out;
    
    if ((ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION)) != Z_OK)
        return ret;
    do {
        deflate(&strm, Z_SYNC_FLUSH);
    } while (strm.avail_in != 0);
    
    write(newsockfd, out, CHUNK - strm.avail_out);

    (void)deflateEnd(&strm);
    return Z_OK;
}

int read_from_shell() {
    int should_exit = 0;
    char buf[BUFFER_SIZE];
    int bytes_read;
    if ((bytes_read = read(child_pipe[READ], buf, sizeof(char) * BUFFER_SIZE)) > 0) {
        if (compress_flag) {
            deflate_to_socket(buf, bytes_read);
        } else {
            write(newsockfd, buf, bytes_read);
        }
    } else if (bytes_read == 0){
        should_exit = 1;
    } else {
        fprintf(stderr, "Could not read from shell\r\n");
        exit(1);
    }
    return should_exit;
}

void parent() {
    close(child_pipe[WRITE]);
    close(parent_pipe[READ]);

    struct pollfd fds[2];

    fds[0].fd = newsockfd;
    fds[0].events = POLLIN;

    fds[1].fd = child_pipe[READ];
    fds[1].events = POLLIN;

    int should_exit = 0;
    while (!should_exit) {
        if (poll(fds, 2, 0) == -1) {
            fprintf(stderr, "Error during poll\r\n");
            close(newsockfd);
            close(sockfd);
            exit(1);
        }
        short error = POLLERR | POLLHUP;
        if (fds[0].revents & POLLIN)
            should_exit = read_from_socket();
        
        if (fds[0].revents & error) {
            fprintf(stderr, "Could not read from socket\r\n");
            should_exit = 1;
            close(newsockfd);
            close(sockfd);
        }
            
        if (fds[1].revents & POLLIN)
            should_exit |= read_from_shell();

        if (fds[1].revents & error) {
            fprintf(stderr, "Could not read from shell\r\n");
            close(newsockfd);
            close(sockfd);
            should_exit = 1;
        }
    }
    shutdown_processing();
}


int main(int argc, char *argv[])
{
    int port_flag;
    struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case 'p':
                portno = atoi(optarg);
                port_flag = 1;
                break;
            case 'c':
                compress_flag = 1;
                break;
            default:
                fprintf(stderr, "Invalid Arguments. Usage: ./lab1b-server --port=<port_number> [--compress] \r\n");
                exit(1);
                break;
        }
    }
     if (!port_flag) {
         fprintf(stderr,"Must provide port for process to run on. Usage: ./lab1b-server --port=<port_number> [--compress --log=<filename>] \r\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
         perror("ERROR on binding");
         exit(1);
     }
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) {
        perror("ERROR on accept");
        exit(1);
     }
     bzero(buffer,256);
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
        close(newsockfd);
        close(sockfd);
        exit(1);
    }
    close(newsockfd);
    close(sockfd);
    exit(0);
}
