//NAME: Aayush Patel
//EMAIL: aayushpatel108@ucla.edu
//UID: 105111196

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <getopt.h>

#include "zlib.h"
#define CHUNK 16384
#define BUFFER_SIZE 256

struct termios saved_attributes;
int sockfd, portno, n;

struct sockaddr_in serv_addr;
struct hostent *server;

int compress_flag;
int log_flag;
int fd_log;


void error(char *msg)
{
    perror(msg);
    exit(0);
}

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

void logger(char *bytes, int num_bytes, int sent_flag) {
    char num_bytes_string[20];
    sprintf(num_bytes_string, "%d", num_bytes);
    if (sent_flag) {
        write(fd_log, "SENT ", sizeof(char)*5);
    } else {
        write(fd_log, "RECEIVED ", sizeof(char)*9);
    }
    int length = strlen(num_bytes_string);
    write(fd_log, num_bytes_string, length);
    write(fd_log, " BYTES: ", sizeof(char)*8);
    write(fd_log, bytes, num_bytes);
    write(fd_log, "\n", sizeof(char));
}

int deflate_to_socket(char* buf, int bytes_in) {
    int ret;
    z_stream strm;
    unsigned char out[CHUNK];
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    (ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION));
    strm.avail_in = bytes_in;
    strm.next_in = (unsigned char *) buf;
    strm.avail_out = CHUNK;
    strm.next_out = out;
    
    if ((ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION)))
        return ret;
    
    do {
        deflate(&strm, Z_SYNC_FLUSH);
    } while (strm.avail_in != 0);
    
    int bytes_used_after_compression = CHUNK - strm.avail_out;
    write(sockfd, out, bytes_used_after_compression);
    
    if (log_flag)
        logger((char *) out, bytes_used_after_compression, 1);
    
    (void)deflateEnd(&strm);
    return Z_OK;
}

int inflate_from_socket(char* buf, int bytes_in) {
    int ret;
    z_stream strm;
    unsigned char out[CHUNK];
//    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = inflateInit(&strm);
    strm.avail_in = bytes_in;
    strm.next_in = (unsigned char *) buf;
    strm.avail_out = CHUNK;
    strm.next_out = out;

    if (ret != Z_OK)
        return ret;
    do {
        inflate(&strm, Z_SYNC_FLUSH);
    } while (strm.avail_in != 0);
    
    int bytes_used_after_decompression = CHUNK - strm.avail_out;
    int i;
    for (i = 0; i < bytes_used_after_decompression; i++) {
        switch (out[i]) {
            case '\r':
            case '\n':
                write(STDOUT_FILENO, "\r\n", sizeof(char)*2);
                break;
            default:
                write(STDOUT_FILENO, &out[i], sizeof(char));
        }
    }
    
    (void)inflateEnd(&strm);
    return Z_OK;
}


int read_from_keyboard() {
    char buf[BUFFER_SIZE];
    int bytes_read;
    int should_exit = 0;
    if ((bytes_read = read(STDIN_FILENO, buf, sizeof(char) * BUFFER_SIZE)) > 0) {
        int i;
        for (i = 0; i < bytes_read; i++) {
            switch (buf[i]) {
                case 0x03:
                    write(STDOUT_FILENO, "^C", sizeof(char) * 2);
                    break;
                case 0x04:
                    write(STDOUT_FILENO, "^D", sizeof(char) * 2);
                    break;
                case '\r':
                case '\n':
                    write(STDOUT_FILENO, "\r\n", sizeof(char) * 2);
                    break;
                default:
                    write(STDOUT_FILENO, &buf[i], sizeof(char));
            }
        }
        if (compress_flag) {
            if (deflate_to_socket(buf, bytes_read) != Z_OK) {
                fprintf(stderr, "Error during deflation\r\n");
                exit(1);
            }
        } else {
            write(sockfd, buf, bytes_read);
            if (log_flag)
                logger(buf, bytes_read, 1);
        }
    } else if (bytes_read == 0) {
        exit(0);
    } else {
        fprintf(stderr, "Could not read from keyboard\r\n");
        should_exit = 1;
        exit(1);
    }
    return should_exit;
}

int read_from_socket() {
    char buf[BUFFER_SIZE];
    int bytes_read;
    int should_exit = 0;
    if ((bytes_read = read(sockfd, buf, sizeof(char) * BUFFER_SIZE)) > 0) {
        if (compress_flag) {
            if (inflate_from_socket(buf, bytes_read) != Z_OK) {
                fprintf(stderr, "Error during inflation. \r\n");
                exit(1);
            }
        } else {
            int i;
            for (i = 0; i < bytes_read; i++) {
                switch (buf[i]) {
                    case '\r':
                    case '\n':
                        write(STDOUT_FILENO, "\r\n", sizeof(char)*2);
                        break;
                    default:
                        write(STDOUT_FILENO, &buf[i], sizeof(char));
                }
            }

        }
        if (log_flag)
            logger(buf, bytes_read, 0);
    } else if (bytes_read == 0) {
        exit(0);
    } else {
        fprintf(stderr, "Could not read from shell\r\n");
        should_exit = 1;
        exit(1);
    }
    return should_exit;
}

void poll_for_input() {
    struct pollfd fds[2];

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    int should_exit = 0;
    while (!should_exit) {

        if (poll(fds, 2, 0) == -1) {
            fprintf(stderr, "Error during poll\r\n");
            exit(1);
        }
        short error = POLLERR | POLLHUP;

        if (fds[0].revents & POLLIN)
            should_exit = read_from_keyboard();
        
        if (fds[0].revents & error) {
            fprintf(stderr, "Could not read from keyboard\r\n");
            should_exit = 1;
        }
        if (fds[1].revents & POLLIN)
            should_exit |= read_from_socket();

        if (fds[1].revents & error) {
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    int port_flag;
    struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"log", required_argument, NULL, 'l'},
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
            case 'l':
                log_flag = 1;
                fd_log = creat(optarg, 0666);
                if (fd_log == -1)
                    fprintf(stderr, "Failure to create/write to file.\r\n");
                break;
            case 'c':
                compress_flag = 1;
                break;
            default:
                fprintf(stderr, "Invalid Arguments. Usage: ./lab1b-client --port=<port_number> [--compress --log=<filename>] \r\n");
                exit(1);
                break;
        }
    }
    if (!port_flag) {
        fprintf(stderr,"Must provide port for process to run on. Usage: ./lab1b-server --port=<port_number> [--compress --log=<filename>] \r\n");
        exit(1);
    }
    set_input_mode();

    char buffer[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\r\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    bzero(buffer,256);
    poll_for_input();
    exit(0);
}
