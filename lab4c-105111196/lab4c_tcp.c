#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

const double R0 = 100000.0;
const int B = 4275; 
mraa_aio_context temp_sensor;
struct timeval clock_time;
struct tm *time_value;
int log_flag = 0;
FILE* log_file;
char scale = 'F';
int period = 1;
int stop_flag = 0;
int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;
char* id;
char* host;

void shutdown_processing() {
    if (gettimeofday(&clock_time, NULL) == -1) {
        fprintf(stderr, "Error getting time\n");
    }
    time_value = localtime(&clock_time.tv_sec);
    dprintf(sockfd, "%02d:%02d:%02d SHUTDOWN\n", time_value->tm_hour, time_value->tm_min, time_value->tm_sec);
    if (log_flag) {
        fprintf(log_file, "%02d:%02d:%02d SHUTDOWN\n", time_value->tm_hour, time_value->tm_min, time_value->tm_sec);
    }
    close(sockfd);
    exit(0);
}

void handle_command(char* buf) {
    if (strncmp(buf, "OFF", sizeof(char)*3) == 0) {
        if (log_flag) {
            fputs("OFF\n", log_file);
            fflush(log_file);
        }
        shutdown_processing();
    } else if (strcmp(buf, "START") == 0) {
        stop_flag = 0;
        if (log_flag) {
            fputs("START\n", log_file);
            fflush(log_file);
        }
    } else if (strcmp(buf, "STOP") == 0) {
        stop_flag = 1;
        if (log_flag) {
            fputs("STOP\n", log_file);
            fflush(log_file);
        }
    } else if (strncmp(buf, "LOG", sizeof(char)*3) == 0) {
        if (log_flag) {
            fputs(buf, log_file);
        }
    } else if (strncmp(buf, "SCALE=", sizeof(char)*6) == 0) {
        scale = buf[6];
        if (log_flag) {
            fprintf(log_file, "SCALE=%c\n", scale);
            fflush(log_file);
        }
    } else if (strncmp(buf, "PERIOD=", sizeof(char)*7) == 0){
        period = atoi(&buf[7]);
        if (log_flag) {
            fprintf(log_file, "PERIOD=%d\n", period);
            fflush(log_file);
        }
    } else {
        fprintf(stderr, "Invalid Command. Exiting\n");
        exit(1);
    }
}

void poll_for_input() {
    char buf[256];
    struct pollfd fds[1];
    int bytes_read;
    fds[0].fd = sockfd;
    fds[0].events = POLLIN | POLLHUP | POLLERR;
    if (poll(fds, 1, 0) < 0) {
        fprintf(stderr, "Error polling for input\n");
        exit(1);
    }
    if (fds[0].revents & POLLIN) {
        if ((bytes_read = read(sockfd, buf, 256)) < 0) {
            fprintf(stderr, "Error reading input.\n");
            exit(1);
        }
        int i;
        int j;
        for (i=0; i<bytes_read; i=j+1) {
            for (j = i; buf[j] != '\n'; j++) {}
            buf[j] = 0;
            handle_command(&buf[i]);
        }
    } 
}

double handle_temperature() {
    int a = mraa_aio_read(temp_sensor);
	double R = 1023.0 / ((double)a) - 1.0;
	R *= R0;
	double temperature = 1.0 / (log(R/R0)/B + 1/298.15) - 273.15;
     if (scale == 'F')
         return temperature * 9/5 + 32;
	return temperature;
}

int main(int argc, char **argv) {
    int c;
    struct option long_options[] = {
        {"period", required_argument, NULL, 'p'},
        {"scale", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"id", required_argument, NULL, 'i'},
        {"host", required_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1){
        switch (c) {
            case 'p':
                period = atoi(optarg);
                if (period == 0) {
                    fprintf(stderr, "Please enter a period greater than 0\n");
                    exit(1);
                }
                break;
            case 's':
                scale = optarg[0];
                if (scale != 'F' && scale != 'C') {
                    fprintf(stderr, "Please enter a valid scale\n");
                    exit(1);
                }
                break;
            case 'l':
                log_file=fopen(optarg, "w+");
                log_flag = 1;
                break;
            case 'i':
                id = optarg;
                break;
            case 'h':
                host = optarg;
                break;
            default:
                fprintf(stderr, "Invalid Arguments\n");
                exit(1);
        }
    }
    portno = atoi(argv[argc-1]);
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\r\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR connecting");
        exit(1);
    }

    dprintf(sockfd, "ID=%s\n", id);
    fprintf(log_file, "ID=%s\n", id);
    
    temp_sensor = mraa_aio_init(1);
    if (temp_sensor == NULL) {
        fprintf(stderr, "Error initializing temperature sensor\n");
        mraa_deinit();
        exit(1);
    }

    time_t next_report_time = 0;

    if (gettimeofday(&clock_time, NULL) == -1) {
        fprintf(stderr, "Error getting time\n");
    }

    while (1) {
        if (gettimeofday(&clock_time, NULL) == -1) {
            fprintf(stderr, "Error getting time\n");
            exit(0);
        }
        if (!stop_flag && clock_time.tv_sec >= next_report_time) {
            double temperature = handle_temperature();
            time_value = localtime(&(clock_time.tv_sec));
            dprintf(sockfd, "%02d:%02d:%02d %.1f\n", time_value->tm_hour, time_value->tm_min, time_value->tm_sec, temperature);
            if (log_flag) 
                fprintf(log_file, "%02d:%02d:%02d %.1f\n", time_value->tm_hour, time_value->tm_min, time_value->tm_sec, temperature);
            next_report_time = clock_time.tv_sec + period;
        }
        poll_for_input();
    }

    if (mraa_aio_close(temp_sensor) != 0) {
        fprintf(stderr, "Error closing temperature sensor.\n");
        exit(1);
    }

    exit(0);
}

